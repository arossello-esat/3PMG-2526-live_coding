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

struct LifeComponent { float value;};

struct ScriptComponent {
    ScriptComponent() {
        luastate_.open_libraries(sol::lib::base);
        LoadState();
    }


    ScriptComponent(const std::string& code) {
        luastate_.open_libraries(sol::lib::base);
        LoadState();
        sol::load_result chunk = state.load(lua_program);
        if(chunk.valid()) chunk();
    }

    LoadCppDefinitions() {
        state.usertype<ECSManager> lua_ecs = state.new_usertype<ECSManager>("ECS",sol::no_constructor);
        state.new_usertype<LifeComponent>("Life",
            sol::no_constructor,
            "value", &LifeComponent::value);
        std::reference_wrapper<ECSManager> recs = std::ref(ecs);
        state.set("ECS",recs);
    }
    sol::state luastate_;
};

void f1(auto v);

template<typename T> void f2(T v);

UpdateLuaSystem(std::vector<ScriptComponent>& scv) {
    for(auto component : scv) {
        sol::protected_function on_frame = componente.state["on_frame"];
        if(on_frame.valid()) {
            sol::protected_function_result  res= on_frame();
        }
    }
    for() // ELIMINAR OBJETOS QUE ALGUN SCRIPT HAYA MARCADO PARA DESTRUCCION
}
int main(int,char**) {

    sol::state state;
    state.open_libraries(sol::lib::base);


    ECSManager ecs;

    int player = ecs.CreateEntity();
    {
        auto ptm = &ECSManager::GetComponent<LifeComponent>;
        ecs->*ptm();
    }

    state.usertype<ECSManager> lua_ecs = state.new_usertype<ECSManager>("ECS",sol::no_constructor);


    auto get_index = [](Context& self,const std::string& key, sol::object value) -> sol::object {
        sol::object result = current_[key];
        if(result.valid()) return result;
        return sol::nil;
    }

    auto get_new_index = [](Context& self,const std::string& key, sol::object value) {
        current_[key] = value;
    }

    sol::object* current;
    state["ctx"] = current;

    auto lua_ecs = state.new_usertype<LuaContext>("Context",
        sol::no_constructor,
        sol::meta_function::index, get_index,
        sol::meta_function::new_index, get_new_index
    );

    for(auto&& [key,value] : type_map) {
            lua_ecs["Get"+ value.name()] = value.GetComponent;
    }
    
    state.new_usertype<LifeComponent>("Life",
        sol::no_constructor,
        "value", &LifeComponent::value
    );

    std::reference_wrapper<ECSManager> recs = std::ref(ecs);
    state.set("ECS",recs);

    sol::load_result chunk = state.load(lua_program);
    if(chunk.valid()) {
        chunk();


        auto shield = state["shield"];
        if(shield.valid()) 
        for(auto componente = componenteList.begin()...) {
            sol::protected_function on_frame = componente.state["on_frame"];
            if(add_life.valid())
               sol::protected_function_result  res= on_frame();
        }
        bool full_life = res;
    }

    return 0;
}






























