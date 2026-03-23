#pragma once

#include <cstdint>
#include <functional>
#include <optional>
#include <span>
#include <stdexcept>
#include <string_view>
#include <typeindex>
#include <vector>

// ---------------------------------------------------------------------------
// Core types
// ---------------------------------------------------------------------------

/// Opaque entity handle.  Generation is packed in the upper 32 bits so stale
/// handles can be detected after the slot is recycled.
struct Entity {
    static constexpr uint64_t kInvalid = 0;

    uint64_t id = kInvalid;

    [[nodiscard]] bool IsValid() const noexcept { return id != kInvalid; }
    [[nodiscard]] uint32_t Index()      const noexcept { return static_cast<uint32_t>(id & 0xFFFF'FFFF); }
    [[nodiscard]] uint32_t Generation() const noexcept { return static_cast<uint32_t>(id >> 32); }

    bool operator==(const Entity&) const noexcept = default;
};

// ---------------------------------------------------------------------------
// Errors
// ---------------------------------------------------------------------------

/// Thrown when an operation is performed on an entity that no longer exists.
struct InvalidEntityError : std::runtime_error {
    explicit InvalidEntityError(Entity e)
        : std::runtime_error("Invalid or destroyed entity"),
          entity(e) {}
    Entity entity;
};

/// Thrown when a component that is not present is accessed.
struct ComponentNotFoundError : std::runtime_error {
    explicit ComponentNotFoundError(std::string_view msg)
        : std::runtime_error(std::string(msg)) {}
};

/// Thrown when registry-level limits are exceeded (e.g. max entities).
struct RegistryCapacityError : std::runtime_error {
    explicit RegistryCapacityError(std::string_view msg)
        : std::runtime_error(std::string(msg)) {}
};

// ---------------------------------------------------------------------------
// IRegistry  –  the heart of the ECS
// ---------------------------------------------------------------------------

/**
 * @brief Pure interface for an Entity-Component-System registry.
 *
 * All methods that can fail on bad input document the exception they throw.
 * Implementations are free to use any storage strategy (archetypes, sparse
 * sets, bitset-indexing, …) as long as they satisfy these contracts.
 */
class IRegistry {
public:
    virtual ~IRegistry() = default;

    // -----------------------------------------------------------------------
    // Entity lifecycle
    // -----------------------------------------------------------------------

    /**
     * Allocate a new entity.
     *
     * @return A valid Entity handle whose IsValid() == true.
     * @throws RegistryCapacityError if the registry has reached its maximum
     *         entity count.
     */
    [[nodiscard]] virtual Entity CreateEntity() = 0;

    /**
     * Destroy an entity and all its components.
     *
     * @param entity  The entity to destroy.
     * @throws InvalidEntityError if entity is invalid or already destroyed.
     *
     * @post  IsAlive(entity) == false.
     * @post  Any component previously accessible via entity is no longer
     *        accessible through that handle, even if the underlying slot is
     *        recycled for a new entity.
     */
    virtual void DestroyEntity(Entity entity) = 0;

    /**
     * Test whether an entity handle refers to a live entity.
     *
     * This is generation-safe: a handle to a destroyed entity whose slot has
     * been reused for a new entity must still return false.
     */
    [[nodiscard]] virtual bool IsAlive(Entity entity) const noexcept = 0;

    /**
     * Return the number of currently live entities.
     */
    [[nodiscard]] virtual std::size_t EntityCount() const noexcept = 0;

    // -----------------------------------------------------------------------
    // Component storage
    // -----------------------------------------------------------------------

    /**
     * Attach a component of type T to entity, constructing it in-place with
     * the provided arguments.
     *
     * If entity already has a component of type T, the existing component is
     * replaced.
     *
     * @tparam T    Component type (must be movable).
     * @param entity  Target entity.
     * @param args    Constructor arguments forwarded to T.
     * @return Reference to the newly stored component.
     *
     * @throws InvalidEntityError if entity is not alive.
     */
    template <typename T, typename... Args>
    T& Emplace(Entity entity, Args&&... args) {
        return *static_cast<T*>(
            EmplaceRaw(entity, typeid(T),
                       [&]() -> void* { return new T(std::forward<Args>(args)...); },
                       [](void* p) { delete static_cast<T*>(p); }));
    }

    /**
     * Remove the component of type T from entity.
     *
     * @throws InvalidEntityError        if entity is not alive.
     * @throws ComponentNotFoundError    if entity does not have a T.
     */
    template <typename T>
    void Remove(Entity entity) {
        RemoveRaw(entity, typeid(T));
    }

    /**
     * Access the component of type T attached to entity.
     *
     * @return Reference to the component.
     * @throws InvalidEntityError        if entity is not alive.
     * @throws ComponentNotFoundError    if entity does not have a T.
     */
    template <typename T>
    [[nodiscard]] T& Get(Entity entity) {
        return *static_cast<T*>(GetRaw(entity, typeid(T)));
    }

    template <typename T>
    [[nodiscard]] const T& Get(Entity entity) const {
        return *static_cast<const T*>(GetRaw(entity, typeid(T)));
    }

    /**
     * Non-throwing component access.
     *
     * @return Pointer to component, or nullptr if absent / entity invalid.
     */
    template <typename T>
    [[nodiscard]] T* TryGet(Entity entity) noexcept {
        return static_cast<T*>(TryGetRaw(entity, typeid(T)));
    }

    template <typename T>
    [[nodiscard]] const T* TryGet(Entity entity) const noexcept {
        return static_cast<const T*>(TryGetRaw(entity, typeid(T)));
    }

    /**
     * Return true iff entity is alive and has a component of type T.
     */
    template <typename T>
    [[nodiscard]] bool Has(Entity entity) const noexcept {
        return HasRaw(entity, typeid(T));
    }

    // -----------------------------------------------------------------------
    // Querying / iteration
    // -----------------------------------------------------------------------

    /**
     * Iterate over every entity that has ALL of the listed component types.
     *
     * The callback receives a reference to each matching entity, plus
     * references to each requested component (in the same order as Ts…).
     *
     * Destroying the current entity inside the callback is defined behaviour;
     * destroying *other* entities is undefined.
     *
     * @tparam Ts   One or more component types to filter on.
     * @param fn    Callable with signature  void(Entity, Ts&...)
     */
    template <typename... Ts, typename Fn>
    void View(Fn&& fn) {
        ViewRaw(
            {typeid(Ts)...},
            [&](Entity e, std::span<void*> ptrs) {
                std::size_t i = 0;
                fn(e, *static_cast<Ts*>(ptrs[i++])...);
            });
    }

    /**
     * Return the number of entities that currently have component type T.
     */
    template <typename T>
    [[nodiscard]] std::size_t Count() const noexcept {
        return CountRaw(typeid(T));
    }

    // -----------------------------------------------------------------------
    // Lifecycle helpers
    // -----------------------------------------------------------------------

    /**
     * Destroy all entities and free all component storage.
     *
     * @post  EntityCount() == 0.
     */
    virtual void Clear() noexcept = 0;

protected:
    // -- type-erased primitives implemented by concrete registries -----------

    virtual void*  EmplaceRaw(Entity,
                              std::type_index,
                              std::function<void*()> factory,
                              std::function<void(void*)> deleter) = 0;

    virtual void   RemoveRaw(Entity, std::type_index) = 0;

    virtual void*  GetRaw(Entity, std::type_index) = 0;
    virtual const void* GetRaw(Entity, std::type_index) const = 0;

    virtual void*  TryGetRaw(Entity, std::type_index) noexcept = 0;
    virtual const void* TryGetRaw(Entity, std::type_index) const noexcept = 0;

    virtual bool   HasRaw(Entity, std::type_index) const noexcept = 0;

    virtual void   ViewRaw(std::vector<std::type_index> types,
                           std::function<void(Entity, std::span<void*>)> fn) = 0;

    virtual std::size_t CountRaw(std::type_index) const noexcept = 0;
};