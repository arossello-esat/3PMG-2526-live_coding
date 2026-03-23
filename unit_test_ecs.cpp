/**
 * ecs_test.cpp  –  Unit tests for IRegistry
 *
 * Teaching objectives illustrated here
 * ─────────────────────────────────────
 *  1.  Test fixture with per-test setUp / tearDown         (EcsTest)
 *  2.  Basic happy-path assertions                         (EntityLifecycle)
 *  3.  Exception-based contract tests                      (ThrowsOn*)
 *  4.  Death tests (EXPECT_DEATH / ASSERT_DEATH)           (EcsDeathTest)
 *  5.  Parameterized tests                                  (MultiComponentTest)
 *  6.  Typed tests over multiple component types           (ComponentStorageTest)
 *  7.  Friend / white-box testing via a mock               (MockRegistry)
 *  8.  GMock expectations and EXPECT_CALL                  (MockCallTest)
 *  9.  Custom matcher                                      (IsAliveEntity)
 * 10.  Scoped helper utilities (SCOPED_TRACE)
 *
 * Build example (adjust paths to your layout):
 *   g++ -std=c++20 ecs_test.cpp -lgtest -lgtest_main -lgmock -pthread -o ecs_test
 */
 
#include "ecs.h"
 
#include <gmock/gmock.h>
#include <gtest/gtest.h>
 
#include <algorithm>
#include <string>
#include <vector>
 
// ============================================================================
// Section 0 – Minimal mock so tests can actually run
// ============================================================================
// A *real* project would link against a real implementation.  Here we create
// the simplest possible in-memory mock so every test below can be compiled
// and executed.  The focus is on the *test patterns*, not the mock itself.
 
#include <any>
#include <map>
#include <unordered_map>
 
class MockRegistry final : public IRegistry {
public:
    // ── Entity lifecycle ─────────────────────────────────────────────────────
 
    Entity CreateEntity() override {
        if (forceCapacityError_) throw RegistryCapacityError("capacity exceeded");
        Entity e;
        e.id = (static_cast<uint64_t>(++generation_) << 32) | ++nextIndex_;
        alive_[e.id] = true;
        return e;
    }
 
    void DestroyEntity(Entity e) override {
        RequireAlive(e);
        alive_.erase(e.id);
        storage_.erase(e.id);
    }
 
    bool IsAlive(Entity e) const noexcept override {
        return alive_.count(e.id) > 0;
    }
 
    std::size_t EntityCount() const noexcept override { return alive_.size(); }
 
    void Clear() noexcept override {
        alive_.clear();
        storage_.clear();
    }
 
    // ── Test-only knobs ───────────────────────────────────────────────────────
    void SimulateCapacityError(bool v) { forceCapacityError_ = v; }
 
protected:
    // ── Type-erased primitives ────────────────────────────────────────────────
 
    void* EmplaceRaw(Entity e,
                     std::type_index ti,
                     std::function<void*()> factory,
                     std::function<void(void*)> /*deleter*/) override {
        RequireAlive(e);
        // For the mock we just store inside a shared_ptr<void> with std::any
        void* raw = factory();
        storage_[e.id][ti] = std::shared_ptr<void>(raw, [](void*) {
            /* leaks intentionally in this toy mock */ });
        return raw;
    }
 
    void RemoveRaw(Entity e, std::type_index ti) override {
        RequireAlive(e);
        auto it = storage_.find(e.id);
        if (it == storage_.end() || !it->second.count(ti))
            throw ComponentNotFoundError("component not found");
        it->second.erase(ti);
    }
 
    void* GetRaw(Entity e, std::type_index ti) override {
        RequireAlive(e);
        auto eit = storage_.find(e.id);
        if (eit == storage_.end() || !eit->second.count(ti))
            throw ComponentNotFoundError("component not found");
        return eit->second.at(ti).get();
    }
 
    const void* GetRaw(Entity e, std::type_index ti) const override {
        if (!alive_.count(e.id)) throw InvalidEntityError(e);
        auto eit = storage_.find(e.id);
        if (eit == storage_.end() || !eit->second.count(ti))
            throw ComponentNotFoundError("component not found");
        return eit->second.at(ti).get();
    }
 
    void* TryGetRaw(Entity e, std::type_index ti) noexcept override {
        auto eit = storage_.find(e.id);
        if (eit == storage_.end() || !eit->second.count(ti)) return nullptr;
        return eit->second.at(ti).get();
    }
 
    const void* TryGetRaw(Entity e, std::type_index ti) const noexcept override {
        auto eit = storage_.find(e.id);
        if (eit == storage_.end() || !eit->second.count(ti)) return nullptr;
        return eit->second.at(ti).get();
    }
 
    bool HasRaw(Entity e, std::type_index ti) const noexcept override {
        auto eit = storage_.find(e.id);
        return eit != storage_.end() && eit->second.count(ti) > 0;
    }
 
    void ViewRaw(std::vector<std::type_index> types,
                 std::function<void(Entity, std::span<void*>)> fn) override {
        for (auto& [eid, comps] : storage_) {
            if (!alive_.count(eid)) continue;
            bool all = std::all_of(types.begin(), types.end(),
                                   [&](auto& t) { return comps.count(t) > 0; });
            if (!all) continue;
            std::vector<void*> ptrs;
            ptrs.reserve(types.size());
            for (auto& t : types) ptrs.push_back(comps.at(t).get());
            Entity e; e.id = eid;
            fn(e, ptrs);
        }
    }
 
    std::size_t CountRaw(std::type_index ti) const noexcept override {
        std::size_t n = 0;
        for (auto& [eid, comps] : storage_)
            if (alive_.count(eid) && comps.count(ti)) ++n;
        return n;
    }
 
private:
    void RequireAlive(Entity e) const {
        if (!alive_.count(e.id)) throw InvalidEntityError(e);
    }
 
    uint32_t nextIndex_   = 0;
    uint32_t generation_  = 0;
    bool     forceCapacityError_ = false;
 
    std::unordered_map<uint64_t, bool> alive_;
    std::unordered_map<uint64_t,
        std::map<std::type_index, std::shared_ptr<void>>> storage_;
};
 
// ============================================================================
// Section 1 – Component types used across tests
// ============================================================================
 
struct Position  { float x, y; };
struct Velocity  { float dx, dy; };
struct Health    { int   hp; };
struct Tag       { std::string name; };
 
// ============================================================================
// Section 2 – Base test fixture  (Teaching point: TEST_F + SetUp / TearDown)
// ============================================================================
 
/**
 * @brief Provides a fresh MockRegistry for each test case.
 *
 * KEY CONCEPT: using a fixture avoids copy-pasting setup code and makes
 * teardown automatic.  Every TEST_F in a fixture gets its own instance.
 */
class EcsTest : public ::testing::Test {
protected:
    void SetUp() override {
        reg = std::make_unique<MockRegistry>();
    }
 
    // TearDown() is called even if the test body throws – similar to RAII.
    void TearDown() override {
        reg.reset();
    }
 
    std::unique_ptr<MockRegistry> reg;
};
 
// ============================================================================
// Section 3 – Entity lifecycle
// ============================================================================
 
// Teaching point: Straightforward happy-path tests with meaningful names.
 
TEST_F(EcsTest, CreateEntity_ReturnsValidHandle) {
    Entity e = reg->CreateEntity();
    EXPECT_TRUE(e.IsValid());
}
 
TEST_F(EcsTest, CreateEntity_IncreasesEntityCount) {
    EXPECT_EQ(reg->EntityCount(), 0u);
    reg->CreateEntity();
    EXPECT_EQ(reg->EntityCount(), 1u);
    reg->CreateEntity();
    EXPECT_EQ(reg->EntityCount(), 2u);
}
 
TEST_F(EcsTest, DestroyEntity_DecrementsCount) {
    Entity e = reg->CreateEntity();
    reg->DestroyEntity(e);
    EXPECT_EQ(reg->EntityCount(), 0u);
}
 
TEST_F(EcsTest, DestroyEntity_MakesEntityDead) {
    Entity e = reg->CreateEntity();
    reg->DestroyEntity(e);
    EXPECT_FALSE(reg->IsAlive(e));
}
 
// Teaching point: Generation safety – a reused slot must not appear alive under
// the old handle.
TEST_F(EcsTest, GenerationSafety_StaleHandleIsNotAlive) {
    Entity old = reg->CreateEntity();
    reg->DestroyEntity(old);
 
    // A real registry may reuse the index.  Whatever comes back must differ.
    Entity fresh = reg->CreateEntity();
    EXPECT_FALSE(reg->IsAlive(old))
        << "Stale handle must remain invalid even after slot reuse";
    EXPECT_TRUE(reg->IsAlive(fresh));
}
 
TEST_F(EcsTest, ClearRegistry_RemovesAllEntities) {
    for (int i = 0; i < 10; ++i) reg->CreateEntity();
    reg->Clear();
    EXPECT_EQ(reg->EntityCount(), 0u);
}
 
// ============================================================================
// Section 4 – Exception-based contract tests
//            (Teaching point: EXPECT_THROW / ASSERT_THROW carry type info)
// ============================================================================
 
TEST_F(EcsTest, ThrowsOnDestroyInvalidEntity) {
    Entity bad;  // default-constructed: id == 0
    EXPECT_THROW(reg->DestroyEntity(bad), InvalidEntityError);
}
 
TEST_F(EcsTest, ThrowsOnDestroyAlreadyDestroyedEntity) {
    Entity e = reg->CreateEntity();
    reg->DestroyEntity(e);
    // Second destroy → stale handle
    EXPECT_THROW(reg->DestroyEntity(e), InvalidEntityError);
}
 
TEST_F(EcsTest, ThrowsOnGetMissingComponent) {
    Entity e = reg->CreateEntity();
    EXPECT_THROW(reg->Get<Position>(e), ComponentNotFoundError);
}
 
TEST_F(EcsTest, ThrowsOnGetComponentFromDeadEntity) {
    Entity e = reg->CreateEntity();
    reg->Emplace<Position>(e, 1.f, 2.f);
    reg->DestroyEntity(e);
    EXPECT_THROW(reg->Get<Position>(e), InvalidEntityError);
}
 
TEST_F(EcsTest, ThrowsOnRemoveMissingComponent) {
    Entity e = reg->CreateEntity();
    EXPECT_THROW(reg->Remove<Health>(e), ComponentNotFoundError);
}
 
TEST_F(EcsTest, ThrowsOnCapacityExceeded) {
    reg->SimulateCapacityError(true);
    EXPECT_THROW(reg->CreateEntity(), RegistryCapacityError);
}
 
// Teaching point: ASSERT_THROW vs EXPECT_THROW
//   ASSERT_THROW aborts the test on failure; useful when subsequent code
//   would dereference a null/bad value if the throw did NOT happen.
TEST_F(EcsTest, AssertThrowVariant_AbortTestIfNoThrow) {
    Entity bad;
    ASSERT_THROW(reg->DestroyEntity(bad), InvalidEntityError);
    // Lines below are only reached if the throw was confirmed.
    SUCCEED() << "Exception correctly raised for invalid entity";
}
 
// ============================================================================
// Section 5 – Death tests
//            (Teaching point: EXPECT_DEATH / ASSERT_DEATH fork a subprocess)
//
// Death tests verify that a function call causes the process to crash (via
// assert, abort, std::terminate, or any unhandled signal).  They are distinct
// from exception tests: the process must *die*, not merely throw.
//
// Naming convention: use the "EcsDeathTest" fixture – GTest uses the "Death"
// suffix to route the test into the correct death-test runner.
// ============================================================================
 
class EcsDeathTest : public EcsTest {};
 
// A helper that wraps a call to simulate a raw assert-based pre-condition
// check (i.e. a coding contract violation – NOT a recoverable error).
// In real code this might be an assert() or __builtin_trap().
static void CallGetOnNullptr(IRegistry* r) {
    // Simulate implementation calling assert(entity.IsValid())
    Entity e; // invalid
    assert(e.IsValid() && "pre-condition violated");
    (void)r;
}
 
// Teaching point: EXPECT_DEATH captures exit on assert() / abort().
TEST_F(EcsDeathTest, AssertFiresOnInvalidEntity) {
#ifdef NDEBUG
    GTEST_SKIP() << "assert() is a no-op in release builds; skipping death test";
#endif
    EXPECT_DEATH(CallGetOnNullptr(reg.get()), "pre-condition violated");
}
 
// Teaching point: ASSERT_DEATH – the test itself aborts if death doesn't happen,
// preventing later code from running in a potentially corrupted state.
TEST_F(EcsDeathTest, AssertDeath_NullRegistryDereference) {
#ifdef NDEBUG
    GTEST_SKIP() << "Not applicable in release builds";
#endif
    IRegistry* null = nullptr;
    ASSERT_DEATH(
        {
            // Simulate a null-registry guard assert
            assert(null != nullptr && "registry must not be null");
        },
        "registry must not be null");
}
 
// Teaching point: Death tests can also cover std::terminate from exceptions
// that escape a noexcept boundary.
static void ViolateNoexcept() noexcept {
    throw std::runtime_error("should never escape noexcept");
}
 
TEST_F(EcsDeathTest, NoexceptViolation_Terminates) {
    EXPECT_DEATH(ViolateNoexcept(), "");
}
 
// ============================================================================
// Section 6 – Component storage tests
// ============================================================================
 
TEST_F(EcsTest, EmplaceAndGet_RoundTrip) {
    Entity e = reg->CreateEntity();
    reg->Emplace<Position>(e, 3.f, 4.f);
 
    auto& p = reg->Get<Position>(e);
    EXPECT_FLOAT_EQ(p.x, 3.f);
    EXPECT_FLOAT_EQ(p.y, 4.f);
}
 
TEST_F(EcsTest, Emplace_ReplacesExistingComponent) {
    Entity e = reg->CreateEntity();
    reg->Emplace<Health>(e, 100);
    reg->Emplace<Health>(e, 50);   // replace
    EXPECT_EQ(reg->Get<Health>(e).hp, 50);
}
 
TEST_F(EcsTest, Has_ReturnsFalseWhenMissing) {
    Entity e = reg->CreateEntity();
    EXPECT_FALSE(reg->Has<Velocity>(e));
}
 
TEST_F(EcsTest, Has_ReturnsTrueAfterEmplace) {
    Entity e = reg->CreateEntity();
    reg->Emplace<Velocity>(e, 1.f, 0.f);
    EXPECT_TRUE(reg->Has<Velocity>(e));
}
 
TEST_F(EcsTest, Remove_ComponentNoLongerPresent) {
    Entity e = reg->CreateEntity();
    reg->Emplace<Tag>(e, "player");
    reg->Remove<Tag>(e);
    EXPECT_FALSE(reg->Has<Tag>(e));
}
 
TEST_F(EcsTest, TryGet_ReturnsNullptrWhenMissing) {
    Entity e = reg->CreateEntity();
    EXPECT_EQ(reg->TryGet<Position>(e), nullptr);
}
 
TEST_F(EcsTest, TryGet_ReturnsPointerWhenPresent) {
    Entity e = reg->CreateEntity();
    reg->Emplace<Position>(e, 7.f, 8.f);
    Position* p = reg->TryGet<Position>(e);
    ASSERT_NE(p, nullptr);
    EXPECT_FLOAT_EQ(p->x, 7.f);
}
 
TEST_F(EcsTest, TryGet_DoesNotThrowOnDeadEntity) {
    Entity e = reg->CreateEntity();
    reg->DestroyEntity(e);
    EXPECT_NO_THROW({
        Position* p = reg->TryGet<Position>(e);
        (void)p;  // expected to be nullptr; no crash
    });
}
 
TEST_F(EcsTest, Count_TracksComponentPopulation) {
    EXPECT_EQ(reg->Count<Health>(), 0u);
    Entity a = reg->CreateEntity();
    Entity b = reg->CreateEntity();
    reg->Emplace<Health>(a, 10);
    EXPECT_EQ(reg->Count<Health>(), 1u);
    reg->Emplace<Health>(b, 20);
    EXPECT_EQ(reg->Count<Health>(), 2u);
    reg->Remove<Health>(a);
    EXPECT_EQ(reg->Count<Health>(), 1u);
}
 
// ============================================================================
// Section 7 – View / query tests
// ============================================================================
 
TEST_F(EcsTest, View_VisitsOnlyEntitiesWithAllComponents) {
    Entity withBoth = reg->CreateEntity();
    Entity withOne  = reg->CreateEntity();
    Entity withNone = reg->CreateEntity();
 
    reg->Emplace<Position>(withBoth, 0.f, 0.f);
    reg->Emplace<Velocity>(withBoth, 1.f, 0.f);
 
    reg->Emplace<Position>(withOne, 0.f, 0.f);
    // withNone has nothing
 
    std::vector<Entity> visited;
    reg->View<Position, Velocity>([&](Entity e, Position&, Velocity&) {
        visited.push_back(e);
    });
 
    ASSERT_EQ(visited.size(), 1u);
    EXPECT_EQ(visited[0], withBoth);
}
 
TEST_F(EcsTest, View_MutatesComponentsInPlace) {
    Entity e = reg->CreateEntity();
    reg->Emplace<Position>(e, 0.f, 0.f);
    reg->Emplace<Velocity>(e, 2.f, 3.f);
 
    reg->View<Position, Velocity>([](Entity, Position& p, Velocity& v) {
        p.x += v.dx;
        p.y += v.dy;
    });
 
    auto& pos = reg->Get<Position>(e);
    EXPECT_FLOAT_EQ(pos.x, 2.f);
    EXPECT_FLOAT_EQ(pos.y, 3.f);
}
 
TEST_F(EcsTest, View_EmptyRegistryVisitsNothing) {
    int calls = 0;
    reg->View<Position>([&](Entity, Position&) { ++calls; });
    EXPECT_EQ(calls, 0);
}
 
// Teaching point: Testing safe self-destruction inside a view callback.
TEST_F(EcsTest, View_DestroyCurrentEntityInsideCallback_IsDefinedBehaviour) {
    Entity e = reg->CreateEntity();
    reg->Emplace<Health>(e, 0);
 
    EXPECT_NO_THROW({
        reg->View<Health>([&](Entity current, Health&) {
            reg->DestroyEntity(current);
        });
    });
 
    EXPECT_FALSE(reg->IsAlive(e));
}
 
// ============================================================================
// Section 8 – Parameterized tests
//            (Teaching point: TEST_P + INSTANTIATE_TEST_SUITE_P)
//
// Here we test that multiple initial health values all survive a round-trip.
// ============================================================================
 
struct HealthParam { int initial; int expected; };
 
class HealthRoundTripTest
    : public EcsTest,
      public ::testing::WithParamInterface<HealthParam> {};
 
TEST_P(HealthRoundTripTest, StoredAndRetrievedCorrectly) {
    auto [initial, expected] = GetParam();
    SCOPED_TRACE("initial hp = " + std::to_string(initial));  // Teaching point: SCOPED_TRACE
 
    Entity e = reg->CreateEntity();
    reg->Emplace<Health>(e, initial);
    EXPECT_EQ(reg->Get<Health>(e).hp, expected);
}
 
INSTANTIATE_TEST_SUITE_P(
    HealthValues,
    HealthRoundTripTest,
    ::testing::Values(
        HealthParam{0,    0},
        HealthParam{1,    1},
        HealthParam{100,  100},
        HealthParam{-1,   -1},   // negative HP is technically valid storage
        HealthParam{INT_MAX, INT_MAX}
    ));
 
// ============================================================================
// Section 9 – Typed tests
//            (Teaching point: TYPED_TEST_SUITE – same test logic, many types)
//
// Verifies that the registry can store and retrieve any trivially movable
// component without data corruption.
// ============================================================================
 
template <typename T>
class ComponentRoundTripTest : public EcsTest {};
 
using PodComponents = ::testing::Types<Position, Velocity, Health>;
TYPED_TEST_SUITE(ComponentRoundTripTest, PodComponents);
 
TYPED_TEST(ComponentRoundTripTest, EmplaceAndRemoveLeaveNoTrace) {
    Entity e = this->reg->CreateEntity();
    this->reg->template Emplace<TypeParam>(e);
    EXPECT_TRUE(this->reg->template Has<TypeParam>(e));
    this->reg->template Remove<TypeParam>(e);
    EXPECT_FALSE(this->reg->template Has<TypeParam>(e));
}
 
// ============================================================================
// Section 10 – Custom GMock matcher
//             (Teaching point: MATCHER_P creates reusable, self-describing
//              matchers that appear in failure messages.)
// ============================================================================
 
MATCHER_P(IsAliveIn, registry,
          std::string(negation ? "is NOT alive in" : "is alive in") + " registry") {
    return registry->IsAlive(arg);
}
 
TEST_F(EcsTest, CustomMatcher_IsAliveIn) {
    Entity alive   = reg->CreateEntity();
    Entity dead    = reg->CreateEntity();
    reg->DestroyEntity(dead);
 
    EXPECT_THAT(alive, IsAliveIn(reg.get()));
    EXPECT_THAT(dead,  ::testing::Not(IsAliveIn(reg.get())));
}
 
// ============================================================================
// Section 11 – SCOPED_TRACE for multi-step diagnostics
//             (Teaching point: SCOPED_TRACE appends context to failure output
//              so you know *which iteration* of a loop failed.)
// ============================================================================
 
TEST_F(EcsTest, ScopedTrace_MultiEntityStress) {
    constexpr int kCount = 20;
    std::vector<Entity> entities;
 
    for (int i = 0; i < kCount; ++i) {
        SCOPED_TRACE("Creating entity " + std::to_string(i));
        entities.push_back(reg->CreateEntity());
        EXPECT_TRUE(entities.back().IsValid());
        EXPECT_TRUE(reg->IsAlive(entities.back()));
    }
 
    EXPECT_EQ(reg->EntityCount(), static_cast<std::size_t>(kCount));
 
    for (int i = 0; i < kCount; ++i) {
        SCOPED_TRACE("Destroying entity " + std::to_string(i));
        reg->DestroyEntity(entities[i]);
        EXPECT_FALSE(reg->IsAlive(entities[i]));
    }
 
    EXPECT_EQ(reg->EntityCount(), 0u);
}
 
// ============================================================================
// Section 12 – Interplay between multiple components on one entity
// ============================================================================
 
TEST_F(EcsTest, MultipleComponents_IndependentStorage) {
    Entity e = reg->CreateEntity();
    reg->Emplace<Position>(e, 1.f, 2.f);
    reg->Emplace<Velocity>(e, 3.f, 4.f);
    reg->Emplace<Health>(e, 75);
    reg->Emplace<Tag>(e, "hero");
 
    EXPECT_FLOAT_EQ(reg->Get<Position>(e).x,  1.f);
    EXPECT_FLOAT_EQ(reg->Get<Velocity>(e).dx, 3.f);
    EXPECT_EQ(reg->Get<Health>(e).hp,          75);
    EXPECT_EQ(reg->Get<Tag>(e).name,           "hero");
 
    // Removing one must not disturb others
    reg->Remove<Velocity>(e);
    EXPECT_FALSE(reg->Has<Velocity>(e));
    EXPECT_FLOAT_EQ(reg->Get<Position>(e).x, 1.f);  // unchanged
}
 
// ============================================================================
// Section 13 – Edge-case: operating on a default-constructed (null) Entity
// ============================================================================
 
TEST_F(EcsTest, DefaultEntity_IsNotValid) {
    Entity e;
    EXPECT_FALSE(e.IsValid());
    EXPECT_FALSE(reg->IsAlive(e));
}
 
TEST_F(EcsTest, DefaultEntity_ThrowsOnComponentAccess) {
    Entity e;
    EXPECT_THROW(reg->Get<Position>(e),    InvalidEntityError);
    EXPECT_THROW(reg->Remove<Position>(e), InvalidEntityError);
}
 
// ============================================================================
// Section 14 – Const-correctness
// ============================================================================
 
TEST_F(EcsTest, ConstGet_WorksOnConstRef) {
    Entity e = reg->CreateEntity();
    reg->Emplace<Health>(e, 42);
 
    const IRegistry& cref = *reg;
    EXPECT_EQ(cref.Get<Health>(e).hp, 42);
}
 
// ============================================================================
// main() – only needed when NOT linking against gtest_main.
// Uncomment if building without -lgtest_main.
// ============================================================================
// int main(int argc, char** argv) {
//     ::testing::InitGoogleTest(&argc, argv);
//     return RUN_ALL_TESTS();
// }