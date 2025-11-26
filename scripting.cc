#include <string>
#include <sol/sol.h>
std::string lua_program = R"(

mode = "attack"

shield = 3

function add_life(player)
    life = ECS.GetLife(player)
    life = life + 1
    print("El valor de life es ahora", life)

    return life >=100
end

)";

struct ECSManager {

    void f();

    template<typename T>
    T& GetComponent(int entity);
};

std::function(void(ESCManager&)) = &ECSManager::f;


struct LifeComponent { float value;};

int main(int,char**) {

    sol::state state;

    auto ptm = &ECSManager::GetComponent<LifeComponent>;

    ECSManager ecs;

    int player = ecs.CreateEntity();
    ecs->*ptm();

    state.usertype<ECSManager> lua_ecs = state.new_usertype<ECSManager>("ECS",
        "GetLife", &ECSManager::GetComponent<LifeComponent>
    );

    for(auto&& [key,value] : type_map) {
            lua_ecs["Get"+ value.name()] = value.GetComponent;
    }
    
    state.new_usertype<LifeComponent>("Life",
        "value", &LifeComponent::value
    );

    std::reference_wrapper<ECSManager> recs = std::ref(ecs);
    state.set("ECS",recs);

    sol::load_result chunk = state.load(lua_program);
    assert(chunk.valid());

    sol::protected_function add_life = state["add_life"];
    state["shield"];
    sol::protected_function_result  res= add_life(player);
    bool full_life = res;

    return 0;
}