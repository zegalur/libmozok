// Copyright 2024 Pavlo Savchuk. Subject to the MIT license.

#pragma once
// ...

#pragma once

#include "app/handler.hpp"
#include "app/command.hpp"

#include <libmozok/mozok.hpp>

namespace mozok {
namespace app {

struct AppOptions {
    bool pauseOnError = false;
    Str printOnOk;
    Str serverName = "mozok_app";
};


/// @brief ...
class App : public MessageProcessor {
    AppOptions _options;
    Result _status;
    Server* _server;
    Str _stdWorldName;
    Vector<EventHandler> _eventHandlers;

private:
    App(const AppOptions& options) noexcept;
    
public:
    [[nodiscard]]
    static App* create(
        const AppOptions& options,
        Result& status
        ) noexcept;
    
    virtual ~App() noexcept;

    Server* getServer() noexcept;
    const Str& getServerName() const noexcept;
    const Result& getCurrentStatus() const noexcept;
    [[nodiscard]] Result newWorld(const Str& worldName) noexcept; 
    //[[nodiscard]] Result setStdWorld(const Str& worldName) noexcept; 
    [[nodiscard]] Result unpause() noexcept;

    [[nodiscard]] Result applyInitAction(
        const Str& worldName,
        const Str& actionName,
        const StrVec& arguments
        ) noexcept;

    [[nodiscard]] Result addEventHandler(const EventHandler& handler) noexcept;
    [[nodiscard]] Result applyDebugCmd(const DebugCmd& cmd) noexcept;
};

}
}
