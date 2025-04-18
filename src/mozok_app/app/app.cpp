#include "app/app.hpp"
#include "app/block.hpp"
#include "app/callback.hpp"
#include "app/handler.hpp"
#include "app/script.hpp"
#include "app/filesystem.hpp"

#include <libmozok/private_types.hpp>
#include <libmozok/error_utils.hpp>
#include <libmozok/server.hpp>

#include <iostream>
#include <ostream>
#include <sstream>
#include <chrono>

namespace mozok {
namespace app {

namespace {

Result errNotImplemented(const Str& what) {
    return Result::Error("App::" + what + " is not yet implemented!");
}

}

App::App(const AppOptions& options) noexcept 
    : _options(options)
    , _currentServer(nullptr)
    , _callback(nullptr)
    , _exit(false)
{ /*empty*/ }

App::~App() noexcept 
{ /* empty */ }


App* App::create(const AppOptions &options, Result &status) noexcept {
    if(status.isError())
        return nullptr;
    
    App* app = new App(options);
    status <<= app->_status;
    if(status.isError())
        return app;

    status <<= QSFParser::parseAndInit(app);
    if(status.isError())
        return app;

    return app;
}

const Result& App::getCurrentStatus() const noexcept {
    return _status;
}

Server* App::getCurrentServer() noexcept {
    return _currentServer.get();
}

Str App::getCurrentPath() const noexcept {
    std::stringstream ss;
    ss << "(";
    for(SIZE_T i = 0; i < _currentPath.size(); ++i) {
        ss << _currentPath[i].first << "/" << _currentPath[i].second;
        if(i != _currentPath.size() - 1)
            ss << ",";
    }
    ss << ")";
    return Str(ss.str());
}

const AppOptions& App::getAppOptions() const noexcept {
    return _options;
}

Str App::getInfo() noexcept {
    if(_currentServer == nullptr) {
        if(_status.isError())
            return _status.getDescription();
        return "READY";
    }

    std::stringstream ss;
    StrVec worlds = _currentServer->getWorlds();

    // List of all created worlds.
    ss << "* Worlds:" << std::endl;
    for(const auto& w : worlds)
        ss << "    - " << w << std::endl;
    ss << std::endl;

    // The full state of each of the world.
    for(const auto& w : worlds) {
        Str saveFile = _currentServer->generateSaveFile(w);
        ss << "* [" << w << "] Full state:" << std::endl;
        ss << saveFile << std::endl;
    }

    return Str(ss.str());
}

Result App::addEventHandler(
        const EventHandler& handler) noexcept {
    if(handler._block._type == DebugBlock::SPLIT) {
        _splitEvents.push_back(_eventHandlers.size());
        int subSplits = 0;
        for(const auto& cmd : handler._block._cmds)
            if(cmd._cmd == DebugCmd::SPLIT)
                ++subSplits;
        _splitsCount.push_back(subSplits);
    }
    _eventHandlers.push_back(handler);
    return Result::OK();
}

Result App::parseAndApplyCmd(const Str& command) noexcept {
    return QSFParser::parseAndApplyCmd(command, this);
}

Result App::applyDebugCmd(const DebugCmd& cmd) noexcept {
    // Some commands uses first argument for the text messages.
    Str msg = "";
    if(cmd._args.size() > 0 && cmd._args.front().type == DebugArg::STR)
        msg = cmd._args.front().str;

    switch(cmd._cmd) {
        case DebugCmd::PAUSE:
            std::cout << "PAUSE: " << msg << std::endl;
            _callback->onPause(this);
            break;
        case DebugCmd::EXIT:
            std::cout << "EXIT: " << msg << std::endl;
            _exit = true;
            break;
        default:
            break;
    }
    return errorNotImplemented(__FILE__, __LINE__, __FUNCTION__);
}

void App::onActionError(
        const Str& worldName, 
        const Str& actionName,
        const StrVec& actionArguments,
        const Result& errorResult
        ) noexcept {
    // empty //
}

void App::onNewMainQuest(
        const Str& worldName, 
        const Str& questName
        ) noexcept {
    // empty //
    std::cout << "NEW_MAIN_QUEST!" << std::endl;
    for(SIZE_T i = 0; i < _eventHandlers.size(); ++i) {
        const auto& eh = _eventHandlers[i];
        if(eh._event != EventHandler::ON_NEW_MAIN_QUEST)
            continue;
        ...
    }
}

void App::onNewSubQuest(
        const Str& worldName, 
        const Str& questName,
        const Str& parentQuestName,
        const int goal
        ) noexcept {
    // empty //
}

void App::onNewQuestState(
        const mozok::Str& worldName, 
        const mozok::Str& questName
        ) noexcept {
    // empty //
}

void App::onNewQuestStatus(
        const Str& worldName, 
        const Str& questName,
        const QuestStatus questStatus
        ) noexcept {
    // empty //
}

void App::onNewQuestPlan(
        const Str& worldName, 
        const Str& questName,
        const StrVec& actionList,
        const Vector<StrVec>& actionArgsList
        ) noexcept {
    // empty //
}

void App::onSearchLimitReached(
        const mozok::Str& worldName,
        const mozok::Str& questName,
        const int searchLimitValue
        ) noexcept {
    // empty //
}
    
void App::onSpaceLimitReached(
        const mozok::Str& worldName,
        const mozok::Str& questName,
        const int searchLimitValue
        ) noexcept {
    // empty //
}

void App::simulateNext() noexcept {
    // Reset event counters to zero for each event handlers.
    _eventCounters.clear();
    _eventCounters.resize(_eventHandlers.size(), 0);

    // Create a new server for this timeline.
    _currentServer = SharedPtr<Server>(
            Server::createServer(_options.serverName, _status));
    if(_status.isError())
        return;

    // Init, using QSF from the options.
    StdFileSystem stdFileSystem;
    _status <<= _currentServer->loadQuestScriptFile(
            &stdFileSystem, 
            _options.scriptFileName, 
            _options.scriptFile, 
            _options.applyInitAction);
    if(_status.isError())
        return;
    
    _currentServer->startWorkerThread();

    bool isWaiting = false;
    auto waitFrom = std::chrono::system_clock::now();
    const auto MAX_WAIT_TIME = std::chrono::milliseconds(
            _options.maxWaitTime_ms);
    const Result WAIT_ERROR = Result::Error(
            getCurrentPath() + 
            " No new messages for an extended period of time." +
            " Wait limit Reached.");
    do {
        // TODO: apply next applicable action.
        //while(questSolver.applyNextApplicableAction(quest_name, server));

        // Try to process the next message.
        if(_currentServer->processNextMessage(*this)) {
            // New message has been processed.
            isWaiting = false;
            continue;
        }

        if(getCurrentStatus().isError())
            break;
        //if(questSolver.isAllQuestsDone() == true)
        //    break;

        if(isWaiting) {
            auto waitDuration = std::chrono::system_clock::now() - waitFrom;
            if(waitDuration > MAX_WAIT_TIME) {
                _status <<= WAIT_ERROR;
                _exit = true;
                break;
            }
        } else {
            isWaiting = true;
            waitFrom = std::chrono::system_clock::now();
        }
    } while(_exit == false);
    _currentServer->stopWorkerThread();
}

Result App::simulate(AppCallback* callback) noexcept {
    _callback = callback;
    do {
        simulateNext();
        if(_status.isError())
            break;
    } while(_alternatives.empty() == false);
    if(_status.isError())
        callback->onError();
    _callback = nullptr;
    return _status;
}

}
}
