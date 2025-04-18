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
    Str scriptFileName = "";
    Str scriptFile = "";
};

/// @brief ...
class App : public MessageProcessor {
    AppOptions _options;
    Result _status;
    EventHandlers _eventHandlers;
    Vector<int> _eventCounters;
    Vector<HandlerId> _splitEvents;
    Vector<int> _splitsCount;
    SharedPtr<Server> _currentServer;
    using Path = Vector<Pair<int, int>>;
    Path _currentPath;
    HashSet<Str> _alternatives;
    AppCallback* _callback;
    bool _exit;

    App(const AppOptions& options) noexcept;
    void simulateNext() noexcept;

public:
    static App* create(const AppOptions& options, Result& status) noexcept;
    virtual ~App() noexcept;

    const AppOptions& getAppOptions() const noexcept;
    Server* getCurrentServer() noexcept;
    Str getCurrentPath() const noexcept;
    const Result& getCurrentStatus() const noexcept;
    Result newWorld(const Str& worldName) noexcept; 
    Str getInfo() noexcept;
    Result addEventHandler(const EventHandler& handler) noexcept;
    Result applyDebugCmd(const DebugCmd& cmd) noexcept;
    Result parseAndApplyCmd(const Str& command) noexcept;
    Result simulate(AppCallback* callback) noexcept;
    
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
            const Str& questName,
            const Str& parentQuestName,
            const int goal
            ) noexcept override;

    void onNewQuestState(
            const mozok::Str& worldName, 
            const mozok::Str& questName
            ) noexcept override;

    void onNewQuestStatus(
            const Str& worldName, 
            const Str& questName,
            const QuestStatus questStatus
            ) noexcept override;

    void onNewQuestPlan(
            const Str& worldName, 
            const Str& questName,
            const StrVec& actionList,
            const Vector<StrVec>& actionArgsList
            ) noexcept override;

    void onSearchLimitReached(
            const mozok::Str& worldName,
            const mozok::Str& questName,
            const int searchLimitValue
            ) noexcept override;
        
    void onSpaceLimitReached(
            const mozok::Str& worldName,
            const mozok::Str& questName,
            const int searchLimitValue
            ) noexcept override;

};

}
}
