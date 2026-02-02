// nlohman

    virtual nlohmann::json serialize_component(Entity entity) const = 0;
    virtual void deserialize_component(Entity entity, const nlohmann::json& j) = 0;

template<typename T, typename U = void>
struct has_serialize_method : std::false_type {};

template<typename T>
struct has_serialize_method<T, std::void_t<decltype(std::declval<const T&>().serialize())>>
    : std::is_same<decltype(std::declval<const T&>().serialize()), nlohmann::json> {};

template<typename T, typename = void>
struct has_deserialize_method : std::false_type {};

template<typename T>
struct has_deserialize_method<T, std::void_t<decltype(T::deserialize(std::declval<const nlohmann::json&>()))>>
    : std::is_same<decltype(T::deserialize(std::declval<const nlohmann::json&>())), T> {};

template<typename T>
constexpr bool is_basic_json_type_v =
    std::is_arithmetic_v<T>|| 
    std::is_same_v<std::remove_cv_t<T>, std::string> ||
    std::is_same_v<std::remove_cv_t<T>, bool>;

template<typename T>
nlohmann::json serialize(const T& value) {
    if constexpr (detail::is_basic_json_type_v<T>) {
        return value;
    } else {
        static_assert(detail::has_serialize_method<T>::value,
            "Type must have 'nlohmann::json serialize() const' method or be a basic type");
        return value.serialize();
    }
}


struct Patata {
    float p;
        nlohmann::json serialize() const {
        return {{"patata", p}};
    }

    static Patata deserialize(const nlohmann::json& j,std::vector<std::string>&position) {
        position.push_back("Patata deseralization");        
        auto p= Patata{j["patata"].get<float>()};
        position.pop_back();
        return p;

    }
    static Patata deserialize(std::istream& in) {
        in.read(reinterpret_cast<char*>(&p),sizeof(p));

        position.push_back("Patata deseralization");        
        auto p= Patata{j["patata"].get<float>()};
        position.pop_back();
        return p;

    }

}

struct Position {
    float x = 0.0f;
    float y = 0.0f;
    Patata p;
    Position() = default;
    Position(float x_, float y_) : x(x_), y(y_) {}

    nlohmann::json serialize() const {
        return {{"x", x}, {"y", y}, {"p", p.serialize() }};
    }

    static Position deserialize(const nlohmann::json& j) {
        auto patata = Patata::deserialize(j["patata"]);
        if(!patata) return nullopt;
        return Position{
            j["x"].get<float>(),
            j["y"].get<float>(),
            std::move(patata.value())
        };
    }

    bool operator==(const Position&) const = default;
};