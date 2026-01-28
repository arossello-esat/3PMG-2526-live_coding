auto vert = """
#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 FragPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
""";

auto frag = """
#version 460 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 objectColor;

void main()
{
    // Ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;
    
    // Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // Specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;
    
    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
}
""";

unsigned int compileShader(const char* source, GLenum type)
{
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    
    // Check for errors
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cout << "Shader compilation failed:\n" << infoLog << std::endl;
    }
    
    return shader;
}

void createShader() {

    auto vertexShader = compileShader(...);

       unsigned int program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    // Check for linking errors
    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cout << "Program linking failed:\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
}
int main(int,char**) {

    auto program = createProgram(...);

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glewInit();

    unsigned int VAO, VBO;
    
    // Create VAO using DSA
    glCreateVertexArrays(1, &VAO);
    
    // Create VBO using DSA
    glCreateBuffers(1, &VBO);
    
    // Upload vertex data to VBO using DSA
    glNamedBufferStorage(VBO, sizeof(vertices), vertices, 0);
    
    // Bind VBO to VAO's binding point 0 using DSA
    glVertexArrayVertexBuffer(VAO, 0, VBO, 0, 6 * sizeof(float));
    
    // Configure position attribute (location 0) using DSA
    glEnableVertexArrayAttrib(VAO, 0);
    glVertexArrayAttribFormat(VAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(VAO, 0, 0);

    // https://github.com/fendevel/Guide-to-Modern-OpenGL-Functions
    // https://juandiegomontoya.github.io/modern_opengl.html

    // Use shader program
    glUseProgram(shaderProgram);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    GLint lightPosLoc = glGetUniformLocation(shaderProgram, "lightPos");

    while(window->shouldClose) {
        // INPUT
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glProgramUniform3fv(shaderProgram, lightPosLoc, 1, glm::value_ptr(lightPos));
        glProgramUniform3fv(shaderProgram, viewPosLoc, 1, glm::value_ptr(cameraPos));
        ...

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    // Cleanup EN LOS DESTRUCTORES!!!
    // BORRAD OPERADORES DE COPIA
    // IMPLEMENTAD OPERADORES DE MOVIMIENTO
    // (EL OPERADOR DE MOVIMIENTO DEBE DE IMPEDIR LA
    // DESTRUCCION POR EL OBJETO MOVIDO)

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

}

struct RenderComponent;
struct TransformComponent;

struct PhongTransparencyRenderSystem {
    template<typename T> using CompL = std::vector<std::optional<T>>;
    PhongRenderSystem();
    ~PhongRenderSystem();
    void operator()(
        const CompL<RenderComponent>& rcl,
        const CompL<TransformComponent>& tcl,
    );

    Program program_;
    ShaderLocation lightpos;
    ShaderLocation model;
    ShaderLocation view;
    ShaderLocation projection;
    ...;
};

PhongTransparencyRenderSystem::PhongTransparencyRenderSystem()
 : program_{Assets::Load("shaders/Phong.prog")}{

    lightpos = program_.getLocation("LightPos");
    ...;
}

PhongTransparencyRenderSystem::~PhongTransparencyRenderSystem() {

}

PhongTransparencyRenderSystem::operator()(
        const CompL<RenderComponent>& rcl,
        const CompL<TransformComponent>& tcl,
        const LightComponent& light,
        const CameraComponent& camera
    ) {

        // BINDEAMOS Y SETEAMOS MAQUINA DE ESTADO
        lightArray // lightArray
        camera // Transformaciones (View Projection)
        program_ // Shaders ->Program (Vertex Fragment)
        for(light : lightArray) {
            if(first_light) {
                glDepthFunc(GL_LESS);
                glDepthMask(GL_TRUE);
                glDisable(GL_BLEND);
            }
            //ACTIVAR ZBUFFER LECTURA/ESCRITURA
            vector<...> transparentCache;
            for(;;){

                if(rc->transparent) {
                    transparentCache.insert(&rc,&tc);
                    continue;
                }
                auto rc = ...;
                auto tc = ...;

                // POR CADA ENTIDAD BINDEAMOS TEXTURA VAO VBO
                rc->Vertices;
                rc->Faces;
                tc->Model; //Transformaciones (Model)
                rc->Texturas
                rc->VAO

                glDraw()
            }

            //ACTIVAR ZBUFFER LECTURA
            // AJUSTAR GLBLEND
            sortDistance(camera,transparentCache);
            for(auto[rc,tc] : transparentCache){

                // POR CADA ENTIDAD BINDEAMOS TEXTURA VAO VBO
                rc->Vertices;
                rc->Faces;
                tc->Model; //Transformaciones (Model)
                rc->Texturas
                rc->VAO
            // RENDER

                glDraw()
            }
            if(first_light) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE); // Additive blending
            glBlendEquation(GL_FUNC_ADD);
            
            // No escribir al depth buffer, pero sí leer de él
            glDepthMask(GL_FALSE);
            glDepthFunc(GL_EQUAL); // Solo renderizar fragmentos en la misma profundidad
            
            }
        }
}


int main(int,char**) {
    auto engine = Engine(...);
    auto window = Window(...);
    //Input
    auto ecs = EntityManager(...);
    // Rellenar de entidades el ecs

    auto render_system = PhongRenderSystem(...);
    while(!window.done()) {
        // INPUT
        //UPDATE





        render_system(ecs.GetRCList(),ecs.GetTCList());


        window.swap()
    }

}