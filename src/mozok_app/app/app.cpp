#include "app/app.hpp"
#include "app/handler.hpp"

#include <libmozok/private_types.hpp>
#include <libmozok/error_utils.hpp>
#include <libmozok/server.hpp>

#include <ostream>
#include <sstream>

namespace mozok {
namespace app {

namespace {

Result errNotImplemented(const Str& what) {
    return Result::Error("App::" + what + " is not yet implemented!");
}

}

App::App(const AppOptions& options) noexcept 
    : _options(options)
    , _server(Server::createServer(_options.serverName, _status))
{ /*empty*/ }

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

Result App::newWorld(const Str& name) noexcept {
    Result res = _server->createWorld(name);
    return res;
}

const AppOptions& App::getAppOptions() const noexcept {
    return _options;
}

Str App::getInfo() noexcept {
    std::stringstream ss;
    StrVec worlds = _server->getWorlds();

    // List of all created worlds.
    ss << "* Worlds:" << std::endl;
    for(const auto& w : worlds)
        ss << "    - " << w << std::endl;
    ss << std::endl;

    // The full state of each of the world.
    for(const auto& w : worlds) {
        Str saveFile = _server->generateSaveFile(w);
        ss << "* [" << w << "] Full state:" << std::endl;
        ss << saveFile << std::endl;
    }

    return Str(ss.str());
}

Result App::applyInitAction(
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

Result App::addEventHandler(
        const EventHandler& handler) noexcept {
    _eventHandlers.push_back(handler);
    return Result::OK();
}

const Str& App::getServerName() const noexcept {
    return _options.serverName;
}

Result App::applyDebugCmd(const DebugCmd& cmd) noexcept {
    return errorNotImplemented(__FILE__, __LINE__, __FUNCTION__);
}

namespace {

Result saveStates(
        Server* server, 
        SplitPoint& out
        ) noexcept {
    Result res;
    StrVec worlds = server->getWorlds();
    for(const auto& w : worlds) {
        const Str sf = server->generateSaveFile(w);
        out.states[w] = sf;
        if(sf.rfind("error", 0) == 0)
            res <<= Result::Error("Invalid save file. File:\n" + sf);
    }
    return res;
}

}

SplitPoint App::rootSplitPoint() noexcept {
    SplitPoint sp;
    _status <<= saveStates(_server, sp);
    for(HandlerId i=0; i < _eventHandlers.size(); ++i)
        sp.handlerFlags.push_back(SplitPoint::OPEN);
    sp.splitEventId = -1;
    sp.nextBranch = 0;
    return sp;
}

SplitPoint App::makeSplitPoint(HandlerId splitEventId) noexcept {
    SplitPoint sp;
    _status <<= saveStates(_server, sp);
    sp.handlerFlags = _splitPointStack.back().handlerFlags;
    sp.reactionQueue = _splitPointStack.back().reactionQueue;
    sp.splitEventId = int(splitEventId);
    sp.nextBranch = 0;
    return sp;
}

Result App::simulate(AppCallback* callback) noexcept {
    _splitPointStack.push_back(rootSplitPoint());

    while(_status.isOk()) {
        //_status <<= performPlanning();
        while(_server->processNextMessage(*this));
    }

    /*while(callback->onPause(this) && _status.isOk()) {
        
    }*/

    if(_status.isError())
        callback->onError();

    return _status;
}

}
}
