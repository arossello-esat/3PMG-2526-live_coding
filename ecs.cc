#include <vector>
#include <memory>
#include <unordered_map>
#include <optional>

struct TransformComponent { };
struct LifeComponent { float life; };
struct AIComponent { float agressiveness; };
struct DrawComponent {
    //&std::shared_ptr<Mesh>
    // Opengl_Mesh_id 
    // std::shared_ptr<Texture>
    bool lit;
    bool cast_shadows;
    
 };

 void RenderSystem(std::vector<DrawComponent>& draw) {
    for(auto& d : draw) {
        d.Draw();
    }
}



void RegenerateHealth(std::vector<LifeComponent>& life) {
    // Bind program
    // bindear VAO, VBO, texturas
    for(auto& l : life) {
        // Poner uniforms
        // pintar
    }
}

void FightOrFlee(std::vector<LifeComponent>& life,std::vector<AIComponent>& ai) {

    auto life_it = life.begin();
    auto ai_it = ai.begin();
    for(; life_it != life.end() && ai_it != ai.end(); life_it++,ai_it++) {

        if(ai_it->agressiveness > life_it->life) attack();
        else flee();
    }
}

void FightOrFlee2(std::vector<std::optional<LifeComponent>>& life,std::vector<std::optional<AIComponent>>& ai) {

    auto life_it = life.begin();
    auto ai_it = ai.begin();
    for(; life_it != life.end() && ai_it != ai.end(); life_it++,ai_it++) {
        if(!ai_it->has_value() || !life_it->has_value()) continue;

        if(ai_it->agressiveness > life_it->life) attack();
        else flee();
    }
}

void FightOrFlee2(std::vector<std::pair<size_t,LifeComponent>>& life,std::vector<std::pair<size_t,AIComponent>>& ai) {

    auto life_it = life.begin();
    auto ai_it = ai.begin();
    for(; life_it != life.end() && ai_it != ai.end(); life_it++,ai_it++) {

        if(ai_it->first != life_it->first) {
            continue;
        }

        if(ai_it->agressiveness > life_it->life) attack();
        else flee();
    }
}
// class ECSListBase {
//     public:
//     virtual void grow() = 0;
//     virtual size_t size() = 0;

// };


// template<typename T>
// class ECSList : public ECSListBase {
//     public:
//     virtual void grow() { list.emplace_back(); }
//     virtual size_t size() { return list.size(); }

//     std::vector<std::optional<T>> list;
// };

// class ECSManager {
//     std::unordered_map<std::size_t,std::unique_ptr<ECSListBase>> component_map_;

//     public:
//     template<typename T> std::vector<T>& get_component_list() {
//         std::size_t hash = typeid(T).hash_code();
//         auto it = component_map_.find(hash);
//         if(it == component_map_.end()) {
//             assert(false && "Component type unknown");
//         }

//         ECSListBase& eclb = *(it->second);
//         ECSList<T>& eclt = static_cast<ECSList<T>&>(eclb);

//         return eclt;
//     }


//     template<typename T>
//         void apply(T call) {
//             call(transform_list);
//         }
//     template<typename T> T* get_component(unsigned long entity) {
//         auto& maybe_component = get_component_list()[entity];
//         if(maybe_component) {
//             return &(maybe_component.value())
//         } else return nullptr;
//     }

//     template<typename T> void AddComponentType() {
//         std::size_t hash = typeid(T).hash_code();
//         using vt = std::unordered_map<std::size_t,std::unique_ptr<ECSListBase>>::value_type;

//         std::unique_ptr<ECSListBase> ecslbptr = std::make_unique<ECSList<T>>(); 

//         component_map_.insert(vt{hash,ecslbptr});

//     }
//     unsigned long AddEntity() {
//         auto size = component_map_.begin()->second->size();
//         for(auto& cl : component_map_ ) {
//             cl.second->grow();
//         }
//         return size;
//     }

//     // Componentes opcionales


//     // Borrar entidades


// };


class ECSListBase {
    public:
    virtual void grow() = 0;
    virtual size_t size() = 0;

};


template<typename T>
class ECSList : public ECSListBase {
    public:
    virtual void grow() { list.emplace_back(); }
    virtual size_t size() { return list.size(); }

    std::vector<std::pair<size_t,T>>
    bool dirty;
};


class ECSManager {
    std::unordered_map<std::size_t,std::unique_ptr<ECSListBase>> component_map_;

    public:
    template<typename T> void AddComponentType();
    
    unsigned long AddEntity();
    void RemoveEntity(unsigned long);

    template<typename T> std::vector<T>& GetComponentList() ;

    template<typename T> T* GetComponent(unsigned long entity);
    template<typename T> T* AddComponent(unsigned long entity);
    template<typename T> void RemoveComponent(unsigned long entity);

    // Componentes opcionales


    // Borrar entidades


};

std::unordered_map<std::size_t,std::unique_ptr<ECSListBase>> component_map_;


template<typename T> std::vector<T>& ECSManager::get_component_list() {
    std::size_t hash = typeid(T).hash_code();
    auto it = component_map_.find(hash);
    if(it == component_map_.end()) {
        assert(false && "Component type unknown");
    }

    ECSListBase& eclb = *(it->second);
    ECSList<T>& eclt = static_cast<ECSList<T>&>(eclb);

    return eclt.list;
}


template<typename T> T* ECSManager::get_component(unsigned long entity) {
    auto& cl = get_component_list()[entity];
    
    std::lower_bound()
}

template<typename T> void ECSManager::AddComponentType() {
    std::size_t hash = typeid(T).hash_code();
    using vt = std::unordered_map<std::size_t,std::unique_ptr<ECSListBase>>::value_type;

    std::unique_ptr<ECSListBase> ecslbptr = std::make_unique<ECSList<T>>(); 

    component_map_.insert(vt{hash,ecslbptr});

}
unsigned long ECSManager::AddEntity() {
    auto size = component_map_.begin()->second->size();
    for(auto& cl : component_map_ ) {
        cl.second->grow();
    }
    return size;
}


int main(int argc, char* argv[]) {

    ECSManager ecs;

    ecs.AddComponentType<LifeComponent>();
    ecs.AddComponentType<TransformComponent>();
    ecs.AddComponentType<AIComponent>();
    ecs.AddComponentType<DrawComponent>();

    // component_map_ = {
    //     {314123,std::unique_ptr<ECSListBase>} -> std::unique_ptr<ECSList<LifeComponent>>.list = {}
    //     {123443,std::unique_ptr<ECSListBase>} -> std::unique_ptr<ECSList<TransformComponent>>.list = {}
    //     {324234,std::unique_ptr<ECSListBase>} -> std::unique_ptr<ECSList<AIComponent>>.list = {}
    //     {234234,std::unique_ptr<ECSListBase>} -> std::unique_ptr<ECSList<DrawComponent>>.list = {}
    // }

    RegenerateHealth(ecs.get_component_list<LifeComponent>());

    unsigned long player = ecs.AddEntity();

    ecs.get_component<LifeComponent>(player).life = 100.0f;

    return 0;
}