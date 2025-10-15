#include <optional>
#include <vector>

class Audio {
    public:
    Audio(int); // Constructor, a secas

    Audio();//Constructor por defecto
    Audio(const Audio& right); // Constructor de copia
    Audio & operator=(const Audio& right); // asignacion de copia

    Audio(Audio& right); // constructor de movimiento
    Audio& operator=(Audio& right); // asignacion de movimiento

    std::vector<float> gigante_; // 100.000 elementos
};

class Window {
public:
    ~Window() {
        if(glfwwindow_) {
            GlfwDestroyWindow(glfwwindow_);
            GLFWTerminate();
        }
    }

    Window(const Window& right) = delete;
    Window & operator=(const Window& right) = delete;

    Window(Window&& right) : glfwwindow_{right.glfwwindow_}{
        right.glfwwindow_ = nullptr;
    }
    Window & operator=(Window&& right) {
        glfwwindow_ = right.glfwwindow_;
        right.glfwwindow_ = nullptr;
    }
 
    static std::optional<Window> create();

    
private:
    Window(GLFWWindow* gw);

    GLFWWindow* glfwwindow_;
};

struct Action{
    Button[] buttons; // SPACE,X...
    Type type; //bool,float,vec2,vec3...
    Action action: //PRESS,RELEASE,DOWN
}
Window a;
Window b = a;

std::optional<Window> Window::create() {

    if(glfwInit()) {
        return std::nullopt;
    }

    GLFWWindow* gw = glfwCreateWindow(640,480,"JuegoGuay");

    if(gw == nullptr) {
        return std::nullopt;
    }

    return Window{gw};
}

class  Input {
        bool IsPressed(Button b) const;
};

class Engine {
    public:
        ~Engine();

        Engine(const Engine& right) = delete;
        Engine& operator=(const Engine& right) = delete;
        
        Engine(Engine& right);
        Engine& operator=(Engine& right);

        bool IsClosing() const;
        void Close();

        void EndFrame();
        
        static std::optional<Engine> create();
    private:
        Engine(Window& window);
        Window window_;
        Audio audio_;
        int mientero_;
};

std::optional<Engine> Engine::create() {
    Window w = Window::create();
    if(!w) return nullopt;
    return std::make_optional(Engine(std::move(w)));
}

Engine::Engine(Window& window) : window_{std::move(window)}, audio_{}, mientero_{3} {
}


class Animation {
    Animation(Input& i) 
    void animwithinput() {

    }

};

enum class DefaultActions {
    Up = GLFW_INPUT_W,
    Down,
    Jump,
};

enum class Actions {
    Up = Input::DefaultActions::Up,
    Down,
    Jump,
};


class Input {
    int a;
    public:

    Input& operator()(int);

    Input(GLFWWindow* w) : window_{w} {
        if(window_input_map_.empty()) {
           glfw_setkeycallback(window_,static_callback);
        }
        Input::window_input_map_.insert(window_,this);
        this->a = 1;
    }
    ~Input() {
        Input::window_input_map_.erase(window_);
    }

    //TODO Implement move
    //TODO delete copy
    GLFWWindow* window_;

    static void static_callback(GLFWWindow*w,int a,int b,int c,int d) {
        window_input_map_[w]->callback(a,b,c,d);

        void (*myp)(int, float); 
    }
    void callback(GLFWWindow*w,int a,int b,int c,int d) {
    }

    static std::map<GLFWWindow*,Input*> window_input_map_;

};

struct millamable {
    float operator()(int a,float b) {
        if(a>4) b *= 45;
        return b;
    }
}

auto main(int, char**) -> int {

    millamable m;
    m(1,2.0f);



    auto milambda = [](int a, float b) -> float {
        if(a>4) b *= 45;
        return b;
    };

    std::vector<int> vi;
    std::sort(vi.begin(),vi.end(),milambda);



    std::optional<Engine> maybe_e = Engine::create();
    if(!maybe_e) return -1;
    
    Engine e =std::move(maybe_e.value());

    Input& i = e.input();

    i.SetDeadzone(0.01,0.01);

    Action jump;
    jump.key = "SPACE";
    
    vec3 player;
    bool direction;
    while(!e.IsClosing()) {
        bool algo;
        if(algo) e.Close();
        if(i.isPressed(ChangeRotationDirection)){
            player += up;
        }

        i.isKeyUp(Jump);
        if(i.isKeyDown("SPACE")){
            direction = !direction;
        }
        player.xy += i.AxisValue();

        
        if(i.JoystickMoving()) {
                                vec2 movement = i.MouseOffset();
             player.xy += i;
        }
        vec2 movement = i.MouseOffset();
        player.xy += movement;
        e.EndFrame();
    }
    return 0;
} 