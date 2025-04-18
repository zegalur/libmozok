// Copyright 2024 Pavlo Savchuk. Subject to the MIT license.

// ...

#pragma once

#include "app/handler.hpp"
#include "app/command.hpp"
#include "app/callback.hpp"

#include <libmozok/public_types.hpp>
#include <libmozok/private_types.hpp>
#include <libmozok/mozok.hpp>

namespace mozok {
namespace app {

struct AppOptions {
    bool pauseOnError = false;
    Str serverName = "mozok_app";
    bool applyInitAction = true;
    int maxWaitTime_ms = 500;
    Str printOnOk = "";
};

struct QuestPlan {

};

struct SplitPoint {
    enum HandlerFlag {
        CLOSED = 0,
        OPEN = 1
    };
    UnorderedMap<Str, Str> states;
    UnorderedMap<Str, QuestPlan> plans;
    Queue<HandlerId> reactionQueue;
    Vector<HandlerFlag> handlerFlags;
    int splitEventId;
    int nextBranch;
};


/// @brief ...
class App : public MessageProcessor {
    AppOptions _options;
    Result _status;
    Server* _server;
    Str _stdWorldName;

    EventHandlers _eventHandlers;
    Vector<SplitPoint> _splitPointStack;

private:
    App(const AppOptions& options) noexcept;
    
    Result performPlanning() noexcept;
    SplitPoint rootSplitPoint() noexcept;
    SplitPoint makeSplitPoint(HandlerId splitEventId) noexcept;

public:
    static App* create(
        const AppOptions& options,
        Result& status
        ) noexcept;
    
    virtual ~App() noexcept;

    Server* getServer() noexcept;
    const Str& getServerName() const noexcept;
    const Result& getCurrentStatus() const noexcept;
    Result newWorld(const Str& worldName) noexcept; 
    const AppOptions& getAppOptions() const noexcept;
    Str getInfo() noexcept;

    Result applyInitAction(
        const Str& worldName,
        const Str& actionName,
        const StrVec& arguments
        ) noexcept;

    Result addEventHandler(const EventHandler& handler) noexcept;
    Result applyDebugCmd(const DebugCmd& cmd) noexcept;

    Result simulate(AppCallback* callback) noexcept;

/*
    void onActionError(
        const Str& worldName, 
        const Str& actionName,
        const StrVec& actionArguments,
        const Result& errorResult
        ) noexcept override;

    void onNewMainQuest(
        const Str& worldName, 
        const Str& questName
        ) noexcept override;

    void onNewSubQuest(
        const Str& worldName, 
        const Str& subquestName,
        const Str& parentQuestName,
        const int goal
        ) noexcept override;
*/


};

}
}
