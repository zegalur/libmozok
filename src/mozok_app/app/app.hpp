// Copyright 2024 Pavlo Savchuk. Subject to the MIT license.

// ...

#pragma once

#include "app/block.hpp"
#include "app/handler.hpp"
#include "app/command.hpp"
#include "app/callback.hpp"

#include <libmozok/message_processor.hpp>
#include <libmozok/public_types.hpp>
#include <libmozok/private_types.hpp>
#include <libmozok/mozok.hpp>

namespace mozok {
namespace app {

struct AppOptions {
    bool pauseOnError = false;
    Str serverName = "mozok_app";
    bool applyInitAction = true;
    int maxWaitTime_ms = 1000;
    Str printOnOk = "";
    Str scriptFileName = "";
    Str scriptFile = "";
    bool verbose = false;
    bool colorText = true;
};

struct QuestRec;
using QuestRecPtr = SharedPtr<QuestRec>;
struct QuestRec {
    const Str worldName;
    const Str questName;
    const bool isMainQuest;
    Vector<QuestRecPtr> subquests;
    QuestStatus lastStatus;
    StrVec lastPlan_Actions;
    Vector<StrVec> lastPlan_Args;
    int nextAction;
    int skippedActions;
    QuestRec(const Str& world, const Str& quest, const bool isMain) noexcept;
};

/// @brief ...
class App : public MessageProcessor {
    AppOptions _options;
    Result _status;
    EventHandlers _eventHandlers;
    HandlerSet _onSearchLimitReached;
    HandlerSet _onSpaceLimitReached;
    HandlerSet _onNewMainQuest;
    HandlerSet _onNewSubQuest;
    HandlerSet _onAction;
    HandlerSet _onInit;
    HandlerSet _onPre;
    Vector<QuestRecPtr> _mainQuests;
    UnorderedMap<Str, QuestRecPtr> _records;
    Vector<HandlerId> _splitEvents;
    Vector<int> _splitsCount;
    SharedPtr<Server> _currentServer;
    using Path = Vector<Pair<int, int>>;
    Path _currentPath;
    UnorderedMap<Str, int> _alternatives;
    HashSet<Str> _donePaths;
    AppCallback* _callback;
    bool _exit;

    App(const AppOptions& options) noexcept;
    Str msg(const Str& text) const noexcept;
    void infoMsg(const Str& msg) const noexcept;
    void errorMsg(const Str& msg) const noexcept;
    void simulateNext() noexcept;
    bool applyNextApplicableAction() noexcept;
    bool applyNext(QuestRecPtr& rec) noexcept;
    bool isAllQuestsDone() noexcept;
    Str pathStr(const Path& path) const noexcept;
    Result applySplitBlock(const DebugBlock& block, int split) noexcept;
    
    void pushAction(
            bool isNA,
            const Str& worldName, 
            const Str& actionName, 
            const StrVec& args
            ) noexcept;

    template<typename ...Args> 
    Result onEvent(HandlerSet& hset, const Args&... args) noexcept;

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
    Result parseAndApplyCmd(const Str& command) noexcept;
    Result applyDebugCmd(const DebugCmd& cmd) noexcept;
    Result applyDebugBlock(const DebugBlock& block) noexcept;
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
