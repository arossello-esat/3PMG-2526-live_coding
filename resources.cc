#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <optional>

class Resources {
public:
    template<typename T> using factory_f = std::optional<T> (*)(std::string_view);
    
    template<typename T> void addFormat(factory_f<T> factory,std::string_view extension);

    template<typename T> std::shared_ptr<T> get(std::string_view name);

    void releaseAll();
private:
    struct FactoryBase { };
    template<typename T> struct FactoryHolder : FactoryBase { factory_f<T> factory; };

    std::unordered_map<std::pair<size_t,std::string>,FactoryBase> factory_map_;
    std::unordered_map<std::pair<size_t,std::string>,std::weak_ptr<void> > instance_map_;
};

class Mesh;
class Texture;
class Shader;
class Program;
class Material;
class AudioClip;
class Level;

Resources resources("c:\\resources\\root\\path");

std::optional<Mesh> LoadGLTF(std::string_view path);
std::optional<Mesh> LoadObj(std::string_view path) {
    // Çarga mallas
    // Encuentra referencia a texturas
    resources.get<Texture>("patata.jpg");
}
std::optional<Texture> LoadPNG(std::string_view path);
std::optional<Texture> LoadJPG(std::string_view path);
std::optional<Shader> LoadVertexShader(std::string_view path);
std::optional<Shader> LoadFragmentShader(std::string_view path);
std::optional<Program> LoadProgram(std::string_view path) {
    auto f = std::fstream(path);
    auto file1 = f.readline();
    auto file2 = f.readline();
    auto vert = resources.get<Shader>(file1);
    auto frag = resources.get<Shader>(file2);

    glLinkProgramLoQueSea(vert,frag);

}
std::optional<AudioClip> LoadOgg(std::string_view path);



int main(int,char**) {
    res.addFormat<Mesh>(&LoadGLTF,"gltf");
    res.addFormat<Mesh>(&LoadObj,"obj");
    res.addFormat<Texture>(&LoadPNG,"png");
    res.addFormat<Texture>(&LoadJPG,"jpg");
    res.addFormat<Texture>(&LoadJPG,"jpeg");


    res.addFormat<Shader>(&LoadVertexShader,"vert");
    res.addFormat<Shader>(&LoadFragmentShader,"frag");
    res.addFormat<Program>(&LoadProgram,"prog");
    res.addFormat<AudioClip>(&LoadOgg,"ogg");

    res.get<Mesh>("patata.obj");
    return 0;
}


