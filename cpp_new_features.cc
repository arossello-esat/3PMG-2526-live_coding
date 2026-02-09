#include<type_traits>
#include<vector>
#include<unordered_map>
#include<string>


struct Base {
    virtual int ll() = 0;
};

struct Derived : Base {
    virtual int ll() override { return 1;}
};

template<typename T>
[[nodiscard]] auto f(T callable) -> std::invoke_t<T()> {
    switch(callable) {
        case 1:
        std::cout << "uno" <<std::endl;
        break;
        case 2:
        std::cout << "dos" <<std::endl;
        [[fallthrough]];
        default:
        std::cout << "otro" <<std::endl;
        break;
    }
    return 1;
}

// Regla de 0
struct a {
    std::shared_ptr<int> pi;
};

// Regla de 3, hay que implementar todas si se implementa alguna
struct b {
    std::shared_ptr<int> pi;

    b(const b&);
    b& operator=(const b&);
    ~b();
};

struct cc;

struct Engine {

    cc& algo;
}
// Regla de 5, si implementas constructor o asignacion de movimiento, tienes que implementar todos
struct b {
    std::shared_ptr<int> pi;
    cc ese_algo;
    Engine e;

    bool movido = false;

    explicit b(int i) : e{ese_algo}{

    }

    operator int() {

    }

    b(const b&);
    b& operator=(const b&);
    b(b&& otro) : e{ese_algo}{
        ese_algo = std::move(otro.ese_algo);

        if(this != &otro) {
            pi = std::move(otro.pi);
            movido = otro.movido; // IMPORTANTE
            otro.movido = true;
        }
    }
    b& operator=(b&&otro) {
        ese_algo = otro.ese_algo;
        // La referencia de engine se actualiza sola;
    }
    ~b() {
        if(!movido) {

        }
    }
};

// NRVO
b func() {
    b ret{};
    return ret;
}

enum class Patata {
    Uno,
    Dos,
    Tres,
    Patata_Cuatro
};

int main(int argc,char**argv) {
    if constexpr(DEBUG== true) {

    } else {

    }
    Patata::Uno;
    {
            std::shared_ptr<void> pi = std::make_shared<Derived>();
    }

    auto myb = func();
    auto i = "hola";
    auto l = [y = i]() mutable {};

    std::vector<int> numeros;
    std::vector<bool> flags;
    std::unordered_map<std::string,int> umsi;

    for(auto&& [key, value] : umsi) {
        
    }

    auto ordenar_por_parejas = [](auto a, auto b) -> int { return 1;/*hago algo con r*/}
    sort(x.begin(),x.end(),ordenar_por_parejas);
)

    int siguiente;

    struct feo : public decltype(l) {

    };

    void(*func)() = l;


}