#include "app.hpp"
#include "libmozok/error_utils.hpp"
#include "libmozok/server.hpp"


namespace mozok {
namespace app {

namespace {

[[nodiscard]] Result errNotImplemented(const Str& what) {
    return Result::Error("App::" + what + " is not yet implemented!");
}

}

App::App(const AppOptions& options) noexcept 
    : _options(options)
    , _server(Server::createServer(_options.serverName, _status))
{ /*empty*/ }

[[nodiscard]]
App* App::create(const AppOptions &options, Result &status) noexcept {
    App* app = new App(options);
    status <<= app->_status;
    return app;
}

App::~App() noexcept {
    delete _server;
}

Server* App::getServer() noexcept {
    return _server;
}

const Result& App::getCurrentStatus() const noexcept {
    return _status;
}

[[nodiscard]] Result App::newWorld(const Str& name) noexcept {
    return _server->createWorld(name);
}

/*[[nodiscard]] Result App::setStdWorld(const Str& worldName) noexcept {
    if(_server->hasWorld(worldName) == false)
        return Result::Error(
                "Can't set `" + worldName + "` as the"
                " standard world name because the world with"
                " this name doesn't exist.");
    if(_stdWorldName != "")
        return Result::Error("Can't set `" + worldName + "` as the"
                " standard world name because standard world name"
                " already set to `" + _stdWorldName + "`.");
    _stdWorldName = worldName;
    return Result::OK();
}*/

[[nodiscard]] Result App::unpause() noexcept {
    return errNotImplemented("unpause");
}

[[nodiscard]] Result App::applyInitAction(
        const Str& worldName,
        const Str& actionName,
        const StrVec& arguments
        ) noexcept {
    if(_server->hasWorld(worldName) == false)
        return errorWorldDoesntExist(_options.serverName, worldName);
    if(_server->getActionStatus(worldName, actionName) != Server::ACTION_APPLICABLE)
        return Result::Error("Invalid action `" + actionName + "`");
    for(const auto& objName : arguments)
        if(_server->hasObject(worldName, objName) == false)
            return errorUndefinedObject(worldName, objName);
    return _server->applyAction(worldName, actionName, arguments);
}


[[nodiscard]] Result App::addEventHandler(
        const EventHandler& handler) noexcept {
    _eventHandlers.push_back(handler);
    return Result::OK();
}

const Str& App::getServerName() const noexcept {
    return _options.serverName;
}

[[nodiscard]] Result App::applyDebugCmd(const DebugCmd& cmd) noexcept {
    return errorNotImplemented(__FILE__, __LINE__, __FUNCTION__);
}

}
}
