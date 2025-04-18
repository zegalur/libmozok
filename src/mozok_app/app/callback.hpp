// ...

#pragma once

namespace mozok {
namespace app {

class App;

class AppCallback {
public:
    virtual ~AppCallback() noexcept;

    // Return `true` is you want to continue the simulation.
    // Return `false` if you want immediately stop the simulation.
    virtual bool onPause(App* app) noexcept = 0;

    virtual void onError() noexcept = 0;
};

}
}
