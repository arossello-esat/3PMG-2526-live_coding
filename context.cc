

class Context {

    template<typename T,int I> Add(T* service);
    template<typename T> T* Get();
    std::unordered_map<size_t,void*> context_;
};

int main () {
    AssetLoader al;
    JobSystem js;
    RenderSystem rs;
    ECSManager ecs;
    LicenseKey lk;

    Context c;
    c.Add(&al);
    c.Add(&js);
    c.Add(&rs);
    c.Add(&ecs);
    c.Add(&lk);

    serialize(ecs,c);

}