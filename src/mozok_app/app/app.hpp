// Copyright 2025 Pavlo Savchuk. Subject to the MIT license.

#pragma once

#include "app/block.hpp"
#include "app/handler.hpp"
#include "app/command.hpp"
#include "app/callback.hpp"

#include <libmozok/message_processor.hpp>
#include <libmozok/private_types.hpp>
#include <libmozok/public_types.hpp>
#include <libmozok/result.hpp>
#include <libmozok/mozok.hpp>

#include <chrono>

namespace mozok {
namespace app {

// ============================= OPTIONS =================================== //

/// @brief Debugger app options.
struct AppOptions {
    
    /// @brief If `true`, pause and show the debug terminal on error.
    bool pauseOnError = false;

    /// @brief Server name, used in error/info messages.
    Str serverName = "mozok_app";

    /// @brief If `true`, debugger will apply the actions from the 
    ///        `init:` block of the input QSF file(s).
    bool applyInitAction = true;

    /// @brief Maximum wait time (in milliseconds) before the debugger throws 
    ///        an error if no new messages are received from the server.
    int maxWaitTime_ms = 5000;

    /// @brief Message to print after a fully successful simulation.
    Str printOnOk = "";

    /// @brief Name of the main script file, used in error messages.
    Str scriptFileName = "";

    /// @brief Main script file source code (text).
    Str scriptFile = "";

    /// @brief If `true`, forced debugger to print more detailed information 
    ///        during the simulation.
    bool verbose = false;

    /// @brief If `true`, forced debugger to output the colored text messages 
    ///        for better readability.
    bool colorText = true;

    /// @brief If non-empty, exports the simulation graph to a file with this name.
    Str exportGraphTo = "";

    /// @brief Graph export visibility flags.
    enum ExportFlags {
        PUSH = 1, /// Push action blocks
        META = 2, /// Meta blocks (PRINT, PAUSE, EXIT).
        EVENT = 4, /// Event blocks.
        EXPECT = 8, /// Expect blocks.
        PLAN = 16, /// Plan Accepted/Changed blocks.
        ACTION_ERROR = 32, /// Action error blocks.
        DETAILS = 64, /// Include details.
        BLOCK = 128 /// Include blocks.
    };

    /// @brief Graph export visibility flags.
    int visibilityFlags = 
            ExportFlags::META | ExportFlags::BLOCK | ExportFlags::EXPECT; 
};

// ============================== RECORD =================================== //

struct QuestRec;
using QuestRecPtr = SharedPtr<QuestRec>;

/// @brief Quest record are used by the `App` to store the information about a
///        quest, activated during a simulation iteration.
struct QuestRec {
    
    /// @brief Quest's world name.
    const Str worldName;

    /// @brief Quest name.
    const Str questName;

    /// @brief Is `true` if this quest a main quest.
    const bool isMainQuest;

    /// @brief If `true`, then we expect this quest ending up `DONE`.
    ///        If `false`, then we expect this quest ending up `UNREACHABLE`.
    bool expectDone;

    /// @brief Index in the `App::_allQuests` array.
    const int recId;

    /// @brief Records of subquests opened during the simulation iteration.
    Vector<QuestRecPtr> subquests;

    /// @brief Last received `QuestStatus` of this quest.
    QuestStatus lastStatus;

    /// @brief Last accepted plan's action list.
    StrVec lastPlan_Actions;

    /// @brief Last accepted plan's arguments list.
    Vector<StrVec> lastPlan_Args;

    /// @brief Last received plan's action list.
    StrVec alternativePlan_Actions;

    /// @brief Last received plan's arguments list.
    Vector<StrVec> alternativePlan_Args;

    /// @brief Index of the next action that is waiting to be applied 
    ///        from the last accepted plan.
    int nextAction;

    /// @brief Counts how many times N/A action have been skipped.
    int skippedActions;

    QuestRec(
            const Str& worldName, 
            const Str& questName, 
            const bool isMainQuest,
            const int recordId
            ) noexcept;
};

// =============================== GRAPH =================================== //

struct GraphNode;
using GraphNodePtr = SharedPtr<GraphNode>;

using TimePoint = std::chrono::time_point<std::chrono::system_clock>;
using Duration_ms = std::chrono::milliseconds;

struct GraphNode {
    enum Type {
        START,
        END,
        PUSH,
        EVENT,
        BLOCK,
        SPLIT,
        META,
        EXPECT,
        ERROR,
        ACTION_ERROR,
        PLAN_ACCEPTED,
        PLAN_CHANGED
    };

    Type type;
    Vector<GraphNodePtr> children;
    GraphNodePtr parent;
    TimePoint currentStart;
    Duration_ms worstDuration;
    Str title;
    StrVec text;
};


// ============================ APPLICATION ================================ //

/// @brief Quest debugging tool. Simulates and debugs the process of solving 
///        non-linear quests. For more information please read 
///        `/docs/debugger.md`.
class App : public MessageProcessor {

    /// @brief Stores the application options.
    AppOptions _options;

    /// @brief Current status.
    Result _status;

    // --------------------------------------------------------------------- //

    /// @defgroup Events Event handles
    /// @{
    EventHandlers _eventHandlers;
    HandlerSet _onSearchLimitReached;
    HandlerSet _onSpaceLimitReached;
    HandlerSet _onNewMainQuest;
    HandlerSet _onNewSubQuest;
    HandlerSet _onNewQuestStatus;
    HandlerSet _onAction;
    HandlerSet _onInit;
    HandlerSet _onPre;

    /// @brief Stores the indexes of event handlers with `SPLIT` block(s).
    Vector<HandlerId> _splitEvents;

    /// @brief For every split block it stores its sub-block count.
    Vector<int> _splitsCount;
    /// @}

    /// @defgroup Quests Quest records
    /// @{
    
    /// @brief Activated main quests.
    Vector<QuestRecPtr> _mainQuests;
    
    /// @brief All activated quests.
    Vector<QuestRecPtr> _allQuests;

    /// @brief "<world>.<quest>" -> quest record.
    UnorderedMap<Str, QuestRecPtr> _records;
    
    // --------------------------------------------------------------------- //
    
    /// @}

    /// @defgroup Paths
    /// @{

    /// @brief Server, used in the current timeline (iteration).
    SharedPtr<Server> _currentServer;

    /// @brief Stores a timeline (path) ID as an array of activated event 
    ///        handlers, in the order they were activated, represented 
    ///        as pairs of (handler ID, sub-block ID).
    using Path = Vector<Pair<int, int>>;

    /// @brief Paths hash function.
    struct PathHash {
        std::size_t operator()(const Path& path) const noexcept;
    };

    /// Converts `Path` into a string representation.
    Str pathStr(const Path& path) const noexcept;
    
    /// @brief Current timeline ID.
    Path _currentPath;

    /// @brief Stores the full log of the current timeline for error reporting.
    Str _pathLog;
    
    /// @brief Alternative paths and, if non-zero, the number of alternative 
    ///        sub-paths.
    HashMap<Path, int, PathHash> _alternatives;

    /// @brief Closed timelines.
    HashSet<Path, PathHash> _donePaths;

    /// @brief Names of activated splits in this timeline (path).
    HashSet<Str> _activeSplits;

    /// @}

    // --------------------------------------------------------------------- //
    
    /// @brief Current callback object.
    AppCallback* _callback;

    /// @brief Immediately closes the app if `true`.
    bool _exit;

    // --------------------------------------------------------------------- //
    
    /// @defgroup Messages
    /// @{
    Str msg(const Str& text) const noexcept;
    void infoMsg(const Str& msg) noexcept;
    void errorMsg(const Str& msg) noexcept;
    /// @}

    // --------------------------------------------------------------------- //
    
    /// @defgroup Simulation Quest solving simulation.
    /// @{
    void simulateNext() noexcept;
    bool applyNextApplicableAction() noexcept;
    bool applyNext(QuestRecPtr& rec) noexcept;
    /// @}

    // --------------------------------------------------------------------- //
    
    /// @defgroup Graph
    /// @{
    
    GraphNodePtr _root;
    GraphNodePtr _cursor;

    void exportGraph() noexcept;

    void recordReset() noexcept;
    void recordStart() noexcept;
    void recordEnd() noexcept;
    void recordError() noexcept;

    void recordSplit(
            const Str& name
            ) noexcept;

    void recordMeta(
            const Str& cmd, 
            const Str& text
            ) noexcept;

    void recordExpect(
            const DebugCmd& cmd
            ) noexcept;

    void recordPush(
            bool isNA,
            const Str& worldName, 
            const Str& actionName, 
            const StrVec& args,
            const int data
            ) noexcept;

    void recordEvent(
            const Str& eventName,
            const Str& worldName,
            const StrVec& args
            ) noexcept;

    void recordEventMatch(
            const EventHandler& handler
            ) noexcept;

    void recordNewPlanAccepted(
            const Str& worldName,
            const Str& questName
            ) noexcept;

    void recordPlanSwitch(
            const Str& worldName,
            const Str& questName
            ) noexcept;
            
    void recordActionError(
            const Str& worldName,
            const Str& actionName,
            const StrVec& actionArguments,
            const Result& errorResult,
            const mozok::ActionError actionError,
            const int data
            ) noexcept;

    void pushNode(const GraphNodePtr& node) noexcept;

    /// @}
    
    // --------------------------------------------------------------------- //
    
    /// @defgroup Expectations
    /// @{
    
    /// @brief Encodes the results of expectations check status.
    enum CheckStatus {

        /// @brief All expectations are met.
        STATUS_DONE,

        /// @brief Some expectation(s) are failed beyond the repair.
        STATUS_FAILED,

        /// @brief Not all the expectations are met.
        STATUS_WAITING
    };

    /// @brief Checks whether all expectations are met.
    /// @return The result of the check.
    CheckStatus checkQuestExpectations() noexcept;

    /// @}

    // --------------------------------------------------------------------- //
    
    Result applySplitBlock(const DebugBlock& block, int split) noexcept;
    Result expectUnreachable(const DebugCmd& cmd) noexcept;
    
    /// @brief Pushes action from a debug command into the action queue.
    void pushAction(const DebugCmd& cmd, const int data) noexcept;

    /// @brief Pushes action into the action queue.
    void pushAction(
            bool isNA,
            const Str& worldName, 
            const Str& actionName, 
            const StrVec& args,
            const int data
            ) noexcept;

    template<typename ...Args> 
    Result onEvent(HandlerSet& hset, const Args&... args) noexcept;

    App(const AppOptions& options) noexcept;

public:
    static App* create(const AppOptions& options, Result& status) noexcept;
    virtual ~App() noexcept;

    const AppOptions& getAppOptions() const noexcept;
    Server* getCurrentServer() noexcept;
    Str getCurrentPath() const noexcept;
    const Result& getCurrentStatus() const noexcept;
    Str getInfo() noexcept;

    Result newWorld(const Str& worldName) noexcept; 
    Result addEventHandler(const EventHandler& handler) noexcept;
    Result parseAndApplyCmd(const Str& command) noexcept;
    Result applyDebugCmd(const DebugCmd& cmd) noexcept;
    Result applyDebugBlock(const DebugBlock& block) noexcept;

    /// @brief Simulates and the process of solving non-linear quests.
    Result simulate(AppCallback* callback) noexcept;

    /// @defgroup MessageProcessor
    /// @{

    void onActionError(
            const Str& worldName, 
            const Str& actionName,
            const StrVec& actionArguments,
            const Result& errorResult,
            const mozok::ActionError actionError,
            const int data
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

    void onNewQuestGoal(
        const Str& worldName,
        const Str& questName,
        const int newGoal,
        const int oldGoal
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
    
    /// @}
};

}
}
