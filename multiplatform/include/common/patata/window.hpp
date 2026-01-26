#include<patata/PlatformWindow.hpp>

class PlatformWindow;


class IWindow {
    public:
    virtual swap() = 0;
    virtual is_finished() = 0;
}

std::unique_ptr<IWindow*> IWindowFactory(PlatformEnum platform);