#include <vector>
#include <memory>

struct TransformComponent { };
struct LifeComponent { float life; };
struct AIComponent { float agressiveness; };
struct DrawComponent { };


void RegenerateHealth(std::vector<LifeComponent>& life) {
    for(auto& l : life) {
        l.life += 0.1f;
    }
}

void FightOrFlee(std::vector<LifeComponent>& life,std::vector<AIComponent>& ai) {

    auto life_it = life.begin();
    auto ai_it = ai.begin();
    for(; life_it != life.end() && ai_it != ai.end(); life_it++,ai_it++) {

        if(ai_it->agressiveness > life_it->life) attack();
    }
}

class ECSListBase {};


template<typename T>
class ECSList : public ECSListBase {
    std::vector<T> list;
};

class ECSManager {
    typeid()
    std::vector<std::unique_ptr<ECSListBase>> list;

    public:
    template<typename T> std::vector<T>& get_list();
    std::vector<LifeComponent>& get_life_list();
    std::vector<AIComponent>& get_ai_list();
    std::vector<DrawComponent>& get_draw_list();

    template<typename T> T& get_component(unsigned long entity);

    template<typename T>
        void apply(T call) {
            call(transform_list);
        }

    template<typename T> void AddComponentType() {
        list.push_back(std::make_unique<T>())
    }
    unsigned long AddEntity() {
        transform_list.emplace_back();
        life_list.emplace_back();
        ai_list.emplace_back();
        draw_list.emplace_back();
        return transform_list.size() -1;
    }
};

int main(int argc, char* argv[]) {

    ECSManager ecs;

    ecs.AddComponentType<LifeComponent>();

    RegenerateHealth(ecs.get_list<LifeComponent>());

    unsigned long player = ecs.AddEntity();

    ecs.get_component<LifeComponent>(player).life = 100.0f;

    return 0;
}