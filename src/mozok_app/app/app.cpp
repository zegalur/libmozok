// Copyright 2025 Pavlo Savchuk. Subject to the MIT license.

#include "app/app.hpp"
#include "app/block.hpp"
#include "app/script.hpp"
#include "app/command.hpp"
#include "app/handler.hpp"
#include "app/callback.hpp"
#include "app/filesystem.hpp"

#include <libmozok/message_processor.hpp>
#include <libmozok/private_types.hpp>
#include <libmozok/error_utils.hpp>
#include <libmozok/server.hpp>

#include <iostream>
#include <ostream>
#include <sstream>
#include <fstream>
#include <string>
#include <chrono>
#include <memory>
#include <map>

namespace mozok {
namespace app {

namespace {
/// @brief Converts world name and quest name into `world.quest` string.
inline Str qname(const Str& w, const Str& q) noexcept {
    return w + Str(".") + q;
}
}

// ============================== QuestRec ================================= //

QuestRec::QuestRec(
        const Str& world, 
        const Str& quest, 
        const bool isMain,
        const int recordId
        ) noexcept :
    worldName(world),
    questName(quest),
    isMainQuest(isMain),
    expectDone(true),
    recId(recordId),
    lastStatus(QuestStatus::MOZOK_QUEST_STATUS_UNKNOWN),
    nextAction(-1),
    skippedActions(0)
{ /* empty */ }

// ============================== PathHash ================================= //

namespace {
/// @brief Old `hash_combine` from boost.
template <class T>
inline void hash_combine(std::size_t& seed, const T& v) noexcept {
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}
}

std::size_t App::PathHash::operator()(const Path& path) const noexcept {
    std::size_t h = 0;
    for (const auto& p : path) {
        hash_combine(h, p.first);
        hash_combine(h, p.second);
    }
    return h;
}

// ================================ App ==================================== //

App::App(const AppOptions& options) noexcept 
    : _options(options)
    , _currentServer(nullptr)
    , _callback(nullptr)
    , _exit(false)
{ /* empty */ }

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

Str App::msg(const Str& text) const noexcept {
    return getCurrentPath() + " " + text;
}

void App::infoMsg(const Str& text) noexcept {
    Str m = msg(Str("INFO: ") + text);
    if(_options.colorText)
        m = msg(Str("\033[96mINFO:\033[0m ") + text);
    _pathLog += m + "\n";
    if(_options.verbose)
        std::cout << m << std::endl;
}

void App::errorMsg(const Str& text) noexcept {
    Str m = msg(Str("ERROR: ") + text);
    if(_options.colorText)
        m = msg(Str("\033[91mERROR:\033[0m ") + text);
    _pathLog += m + "\n";
    if(_options.verbose)
        std::cout << m << std::endl;
}

const Result& App::getCurrentStatus() const noexcept {
    return _status;
}

Server* App::getCurrentServer() noexcept {
    return _currentServer.get();
}

Str App::getCurrentPath() const noexcept {
    return pathStr(_currentPath);
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
    ss << std::endl;

    // Current path's log.
    ss << "* Timeline LOG:" << std::endl << _pathLog << std::endl;

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
    // Some commands uses first argument for long text messages.
    Str m = "";
    if(cmd._args.size() > 0 && cmd._args.front().type == DebugArg::STR)
        m = cmd._args.front().str;

    const Str pre = (_options.colorText ? "\033[93m" : "");
    const Str post = (_options.colorText ? "\033[0m" : "");

    switch(cmd._cmd) {
        case DebugCmd::PRINT:
            std::cout << msg(pre + "PRINT: " + post + m) << std::endl;
            recordMeta("PRINT", m);
            break;
        case DebugCmd::PAUSE:
            std::cout << msg(pre + "PAUSE: " + post + m) << std::endl;
            recordMeta("PAUSE", m);
            if(_callback->onPause(this) == false)
                _exit = true;
            break;
        case DebugCmd::EXIT:
            std::cout << msg(pre + "EXIT: " + post + m) << std::endl;
            recordMeta("EXIT", m);
            _exit = true;
            break;
        case DebugCmd::PUSH:
            pushAction(cmd, -1);
            break;
        case DebugCmd::EXPECT:
            recordExpect(cmd);
            if(cmd._questEvent != DebugCmd::UNREACHABLE)
                return errorNotImplemented(__FILE__, __LINE__, __FUNCTION__);
            return expectUnreachable(cmd);
        default:
            return errorNotImplemented(__FILE__, __LINE__, __FUNCTION__);
    }
    return Result::OK();
}

Result App::applyDebugBlock(const DebugBlock& block) noexcept {
    infoMsg("Apply block `" + block._name + "`.");
    Result res;
    for(const auto& cmd : block._cmds) {
        res <<= applyDebugCmd(cmd);
        if(res.isError())
            break;
        if(_exit)
            break;
    }
    return res;
}

Result App::expectUnreachable(const DebugCmd& cmd) noexcept {
    const Str& worldName = cmd._args[0].str;
    const Str& questName = cmd._args[1].str;
    const Str name = qname(worldName, questName);
    if(_records.find(name) == _records.end())
        return errorUndefinedQuest(worldName, questName);
    _records[name]->expectDone = false;
    // info message
    const Str pre = (_options.colorText ? "\033[96m" : "");
    const Str post = (_options.colorText ? "\033[0m" : "");
    infoMsg(pre + "Expect quest `[" + worldName + "] " + questName 
            + "` to be unreachable." + post);
    return Result::OK();
}

Result App::applySplitBlock(const DebugBlock& block, int split) noexcept {
    const Str splitBlockName = block._cmds[block._splits[split]]._args[0].str;
    
    // Info message
    const Str pre = (_options.colorText ? "\033[92m\033[4m" : "");
    const Str post = (_options.colorText ? "\033[0m" : "");
    infoMsg(pre + "-=> Apply split block `" + splitBlockName + "`." + post);

    recordSplit(splitBlockName);

    Result res;
    int start = block._splits[split] + 1;
    int end = static_cast<int>(block._cmds.size());
    if(split < int(block._splits.size())-1)
        end = block._splits[split + 1];
    for(int i = start; i < end; ++i) {
        res <<= applyDebugCmd(block._cmds[SIZE_T(i)]);
        if(res.isError())
            break;
    }
    _activeSplits.insert(splitBlockName);
    return res;

}

Str App::pathStr(const Path& path) const noexcept {
    std::stringstream ss;
    ss << "(";
    for(SIZE_T i = 0; i < path.size(); ++i) {
        ss << path[i].first + 1 << "/" << path[i].second + 1;
        if(i != path.size() - 1)
            ss << ",";
    }
    ss << ")";
    return Str(ss.str());

}

// ----------------------------- EVENTS ------------------------------------ //

template<typename ...Args>
Result App::onEvent(
        HandlerSet& hset, 
        const Args&... args
        ) noexcept {
    Result res;
    for(auto it = hset.begin(); _exit == false && it != hset.end(); ) {
        const HandlerId hid = *it;
        const auto& h = _eventHandlers[hid];
        if(match(h._args, args...)) {
            bool removeHandler = true;
            if(h._block._type == DebugBlock::SPLIT) {
                // Split block.
                Path split = _currentPath;
                split.push_back({static_cast<int>(hid), 0});
                for(SIZE_T i=0; i<h._block._splits.size(); ++i) {
                    if(_alternatives.find(split) != _alternatives.end()) {
                        // This split is among the alternatives. 
                        // Let's do the split.
                        _currentPath = split;
                        res <<= applySplitBlock(h._block, int(i));
                        break;
                    } if(_donePaths.find(split) == _donePaths.end()) {
                        // Make this path a new possible alternative.
                        _alternatives[split] = 0;
                        Path parent = split;
                        do {
                            parent.pop_back();
                            _alternatives[parent]++;
                        } while(parent.size() > 0);
                    }
                    split.back().second++;
                }
            } else if(h._block._type == DebugBlock::ACT_IF) {
                if(_activeSplits.find(h._block._name) != _activeSplits.end()) {
                    // The required split is activated. Apply the block.
                    res <<= applyDebugBlock(h._block);
                } else
                    removeHandler = false;
            } else {
                recordEventMatch(h);
                // Not a split block.
                res <<= applyDebugBlock(h._block);
            }

            if(removeHandler && h._block._type != DebugBlock::ALWAYS)
                it = hset.erase(it);
            else
                ++it;
        } else 
            ++it;
    }
    return res;
}

void App::onActionError(
        const Str& worldName, 
        const Str& actionName,
        const StrVec& actionArguments,
        const Result& errorResult,
        const mozok::ActionError actionError,
        const int data
        ) noexcept {
    Str text = "onActionError: [" + worldName + "]";
    text += actionName + "(";
    for(SIZE_T i=0; i<actionArguments.size(); ++i)
            text += actionArguments[i] 
                    + (i != actionArguments.size()-1 ? "," : "");
    text += "). Error result = `" + errorResult.getDescription() + "`";

    recordActionError(
        worldName, actionName, actionArguments, errorResult, actionError, data);

    if(data < 0 || actionError != MOZOK_AE_PRECONDITIONS_ERROR) {
        _exit = true;
        errorMsg(text);
    } else {
        // Try the alternative plane.
        QuestRecPtr q = _allQuests[data];
        infoMsg("Invalid action from the `" 
                + qname(q->worldName, q->questName) + "` plan.");

        if(q->alternativePlan_Actions.size() > 0) {
            if(q->alternativePlan_Actions.front() == actionName)
                if(q->alternativePlan_Args.front() == actionArguments) {
                    // Alternative plan starts from the same action as last plan.
                    infoMsg("No good alternative plan avaiable for `" 
                            + q->questName + "`. Throwing an error...");
                    _exit = true;
                    errorMsg(text);
                    return;
                }
        }

        infoMsg("Switch to the alternative plan for `" + q->questName + "`.");
        q->lastPlan_Actions = q->alternativePlan_Actions;
        q->lastPlan_Args = q->alternativePlan_Args;
        q->nextAction = 0;
        recordPlanSwitch(worldName, q->questName);
    }
}

void App::onNewMainQuest(
        const Str& worldName, 
        const Str& questName
        ) noexcept {
    infoMsg("EVENT: onNewMainQuest [" + worldName + "] " + questName);
    recordEvent("onNewMainQuest", worldName, {questName});
    QuestRecPtr rec(new QuestRec(worldName, questName, true, int(_records.size())));
    _mainQuests.push_back(rec);
    _records[qname(worldName, questName)] = rec;
    _allQuests.push_back(rec);
    _status <<= onEvent(_onNewMainQuest, worldName, questName);
}

void App::onNewSubQuest(
        const Str& worldName, 
        const Str& questName,
        const Str& parentQuestName,
        const int goal
        ) noexcept {
    infoMsg("EVENT: onNewSubQuest [" + worldName + "] " 
            + questName + " " + parentQuestName + " " + std::to_string(goal));
    recordEvent(
            "onNewSubQuest", worldName, 
            {questName, parentQuestName, std::to_string(goal)});
    QuestRecPtr rec(new QuestRec(worldName, questName, true, int(_records.size())));
    _records[qname(worldName, questName)] = rec;
    _records[qname(worldName, parentQuestName)]->subquests.push_back(rec);
    _allQuests.push_back(rec);
    _status <<= onEvent(
            _onNewSubQuest, worldName, questName, parentQuestName, goal);
}

void App::onNewQuestState(
        const mozok::Str& worldName, 
        const mozok::Str& questName
        ) noexcept {
    infoMsg("EVENT: onNewQuestState [" + worldName + "] " + questName);
    //
    // empty //
    //
}

void App::onNewQuestStatus(
        const Str& worldName, 
        const Str& questName,
        const QuestStatus questStatus
        ) noexcept {
    // Info message.
    Str statusStr = questStatusToStr_Short(questStatus);
    Str statusStrCopy = statusStr;
    Str doneStr = questStatusToStr_Short(QuestStatus::MOZOK_QUEST_STATUS_DONE);
    Str failStr = questStatusToStr_Short(QuestStatus::MOZOK_QUEST_STATUS_UNREACHABLE);
    if(_options.colorText && statusStr == doneStr)
        statusStr = "\033[92m" + statusStr + "\033[0m";
    if(_options.colorText && statusStr == failStr)
        statusStr = "\033[91m" + statusStr + "\033[0m";
    infoMsg("EVENT: onNewQuestStatus [" + worldName + "] " 
            + questName + " " + statusStr);
    if(statusStrCopy == doneStr)
        statusStrCopy = "<font color='darkgreen'><b>" + statusStrCopy + "</b></font>";
    if(statusStrCopy == failStr)
        statusStrCopy = "<font color='red'><b>" + statusStrCopy + "</b></font>";
   
    recordEvent("onNewQuestStatus", worldName, {questName, statusStrCopy});

    _records[qname(worldName, questName)]->lastStatus = questStatus;
    _status <<= onEvent(
            _onNewQuestStatus, worldName, 
            questName, questStatusToStr(questStatus));
}

void App::onNewQuestGoal(
        const Str& worldName,
        const Str& questName,
        const int newGoal,
        const int oldGoal
        ) noexcept {
    std::stringstream ss;
    ss << "EVENT: onNewQuestGoal [" << worldName << "] " << questName << " ";
    ss << newGoal << " " << oldGoal;
    infoMsg(ss.str());

    recordEvent(
            "onNewQuestGoal", worldName, 
            {questName,std::to_string(newGoal),std::to_string(oldGoal)});

    // Reset the plan when goal changed.
    QuestRecPtr rec = _records[qname(worldName, questName)];
    rec->nextAction = -1;
}

void App::onNewQuestPlan(
        const Str& worldName, 
        const Str& questName,
        const StrVec& actionList,
        const Vector<StrVec>& actionArgsList
        ) noexcept {
    infoMsg("EVENT: onNewQuestPlan [" + worldName + "] " + questName);
    QuestRecPtr &rec = _records[qname(worldName, questName)];
    if(rec->nextAction < 0) {
        // accept the plan
        infoMsg("       New plan accepted for ["+worldName+"] "+questName);
        rec->nextAction = 0;
        rec->lastPlan_Actions = actionList;
        rec->lastPlan_Args = actionArgsList;
        recordNewPlanAccepted(worldName, questName);
    }
    rec->alternativePlan_Actions = actionList;
    rec->alternativePlan_Args = actionArgsList;
}

void App::onSearchLimitReached(
        const mozok::Str& worldName,
        const mozok::Str& questName,
        const int limitValue
        ) noexcept {
    infoMsg("EVENT: onSearchLimitReached [" + worldName + "] " + questName);
    recordEvent(
            "onSearchLimitReached", worldName, 
            {questName, std::to_string(limitValue)});
    _status <<= onEvent(
            _onSearchLimitReached, worldName, questName);
}
    
void App::onSpaceLimitReached(
        const mozok::Str& worldName,
        const mozok::Str& questName,
        const int limitValue
        ) noexcept {
    infoMsg("EVENT: onSpaceLimitReached [" + worldName + "] " + questName);
    recordEvent(
            "onSpaceLimitReached", worldName, 
            {questName, std::to_string(limitValue)});
    _status <<= onEvent(
            _onSpaceLimitReached, worldName, questName);
}

// ----------------------------- GRAPH ------------------------------------- //

namespace {
const Str NTITLE_START = "START";
const Str NTITLE_END = "END";
const Str NTITLE_ERROR = "ERROR";
const Str NTITLE_META = "META";
const Str NTITLE_SPLIT = "SPLIT";
const Str NTITLE_EXPECT = "EXPECT";
const Str NTITLE_PUSH = "PUSH";
const Str NTITLE_BLOCK = "BLOCK";
const Str NTITLE_PLAN_ACCEPTED = "PLAN_ACCEPTED";
const Str NTITLE_PLAN_CHANGED = "PLAN_CHANGED";
const Str NTITLE_ACTION_ERROR = "ACTION_ERROR";
}

void App::recordReset() noexcept {
    _root.reset(new GraphNode());
    _root->type = GraphNode::Type::START;
    _root->currentStart = std::chrono::system_clock::now();
    _root->worstDuration = Duration_ms(0);
    _root->title = NTITLE_START;
    _root->text.push_back("File: "+_options.scriptFileName);
    _root->text.push_back("Init: "+Str(_options.applyInitAction?"TRUE":"FALSE"));
    _cursor = _root;
}

void App::recordStart() noexcept {
    _cursor = _root;
    _root->currentStart = std::chrono::system_clock::now();
}

void App::recordEnd() noexcept {
    GraphNodePtr node = makeShared<GraphNode>();
    node->type = GraphNode::Type::END;
    node->title = NTITLE_END;
    pushNode(node);
    //
    // TODO: merge if necessary.
    //
}

void App::recordError() noexcept {
    GraphNodePtr node = makeShared<GraphNode>();
    node->type = GraphNode::Type::ERROR;
    node->title = NTITLE_ERROR;
    node->text.push_back("Status:");
    node->text.push_back(_status.getDescription());
    pushNode(node);
}

void App::recordMeta(
        const Str& cmd, 
        const Str& text
        ) noexcept {
    GraphNodePtr node = makeShared<GraphNode>();
    node->type = GraphNode::Type::META;
    node->title = NTITLE_META;
    node->text.push_back(cmd);
    node->text.push_back(text);
    pushNode(node);
}

void App::recordExpect(
        const DebugCmd& cmd
        ) noexcept {
    GraphNodePtr node = makeShared<GraphNode>();
    node->type = GraphNode::Type::EXPECT;
    node->title = NTITLE_EXPECT;
    node->text.push_back(cmd.questEventStr());
    for(const auto& a : cmd.args())
        node->text.push_back(a.toStr());
    pushNode(node);
}

void App::recordPush(
        bool isNA,
        const Str& worldName, 
        const Str& actionName, 
        const StrVec& args,
        const int data
        ) noexcept {
    GraphNodePtr node = makeShared<GraphNode>();
    node->type = GraphNode::Type::PUSH;
    node->title = NTITLE_PUSH;
    if(data >= 0)
        node->text.push_back("(by " + _allQuests[data]->questName + ")");
    if(isNA)
        node->text.push_back("N/A");
    node->text.push_back("[" + worldName + "]");
    node->text.push_back(actionName);
    node->text.insert(node->text.end(), args.begin(), args.end());
    pushNode(node);
}

void App::recordEvent(
        const Str& eventName,
        const Str& worldName,
        const StrVec& args
        ) noexcept {
    GraphNodePtr node = makeShared<GraphNode>();
    node->type = GraphNode::Type::EVENT;
    node->title = eventName;
    node->text.push_back("[" + worldName + "] " + args.front());
    node->text.insert(node->text.end(), args.begin() + 1, args.end());
    pushNode(node);
}

void App::recordEventMatch(
        const EventHandler& handler
        ) noexcept {
    GraphNodePtr node = makeShared<GraphNode>();
    node->type = GraphNode::Type::BLOCK;
    node->title = NTITLE_BLOCK;
    node->text.push_back(
        "<b>" + handler._block.typeToStr() + " </b>" + handler._block._name);
    pushNode(node);
}

void App::recordNewPlanAccepted(
        const Str& worldName,
        const Str& questName
        ) noexcept {
    GraphNodePtr node = makeShared<GraphNode>();
    node->type = GraphNode::Type::PLAN_ACCEPTED;
    node->title = NTITLE_PLAN_ACCEPTED;
    node->text.push_back("[" + worldName + "]");
    node->text.push_back(questName);
    pushNode(node);
}

void App::recordPlanSwitch(
        const Str& worldName,
        const Str& questName
        ) noexcept {
    GraphNodePtr node = makeShared<GraphNode>();
    node->type = GraphNode::Type::PLAN_CHANGED;
    node->title = NTITLE_PLAN_CHANGED;
    node->text.push_back("[" + worldName + "]");
    node->text.push_back(questName);
    pushNode(node);
}
        
void App::recordActionError(
        const Str& worldName,
        const Str& actionName,
        const StrVec& args,
        const Result& /*errorResult*/,
        const mozok::ActionError actionError,
        const int data
        ) noexcept {
    GraphNodePtr node = makeShared<GraphNode>();
    node->type = GraphNode::Type::ACTION_ERROR;
    node->title = NTITLE_ACTION_ERROR;
    if(data >= 0)
        node->text.push_back("(by " + _allQuests[data]->questName + ")");
    node->text.push_back(actionErrorToStr(actionError));
    //node->text.push_back("Result: " + errorResult.getDescription());
    node->text.push_back("[" + worldName + "]" + actionName);
    node->text.insert(node->text.end(), args.begin(), args.end());
    pushNode(node);
}

void App::recordSplit(
        const Str& name
        ) noexcept {
    GraphNodePtr node = makeShared<GraphNode>();
    node->type = GraphNode::Type::SPLIT;
    node->title = NTITLE_SPLIT;
    node->text.push_back(name);
    pushNode(node);

}

void App::pushNode(const GraphNodePtr& node) noexcept {
    node->currentStart = std::chrono::system_clock::now();
    Duration_ms d = std::chrono::duration_cast<Duration_ms>(
            node->currentStart - _cursor->currentStart);
    node->worstDuration = d;//std::max(d, node->worstDuration);

    // Check if we have the same node among the children.
    for(const GraphNodePtr& ch : _cursor->children) {
        if(ch->type != node->type)
            continue;
        if(ch->title != node->title)
            continue;
        if(ch->text != node->text)
            continue;
        // Identical node was found.
        // Update the worst duration value.
        ch->worstDuration = std::max(ch->worstDuration, node->worstDuration);
        ch->currentStart = node->currentStart;
        _cursor = ch;
        return;
    }

    // Visibility.
    bool visible = false;
    switch(node->type) {
        case GraphNode::Type::START:
            visible = true;
            break;
        case GraphNode::Type::END:
            visible = true;
            break;
        case GraphNode::Type::PUSH:
            visible = _options.visibilityFlags 
                    & (int)(AppOptions::ExportFlags::PUSH);
            break;
        case GraphNode::Type::EVENT:
            visible = _options.visibilityFlags 
                    & (int)(AppOptions::ExportFlags::EVENT);
            break;
        case GraphNode::Type::BLOCK:
            visible = _options.visibilityFlags 
                    & (int)(AppOptions::ExportFlags::BLOCK);
            break;
        case GraphNode::Type::SPLIT:
            visible = true;
            break;
        case GraphNode::Type::META:
            visible = _options.visibilityFlags 
                    & (int)(AppOptions::ExportFlags::META);
            break;
        case GraphNode::Type::EXPECT:
            visible = _options.visibilityFlags 
                    & (int)(AppOptions::ExportFlags::EXPECT);
            break; 
        case GraphNode::Type::ERROR:
            visible = true;
            break;
        case GraphNode::Type::ACTION_ERROR:
            visible = _options.visibilityFlags 
                    & (int)(AppOptions::ExportFlags::ACTION_ERROR);
            break;
        case GraphNode::Type::PLAN_ACCEPTED:
            visible = _options.visibilityFlags 
                    & (int)(AppOptions::ExportFlags::PLAN);
            break;
        case GraphNode::Type::PLAN_CHANGED:
            visible = _options.visibilityFlags 
                    & (int)(AppOptions::ExportFlags::PLAN);
            break;
        default:
            break;
    }

    if(visible == false) {
        // This node isn't visible.
        // Only update the timings.
        _cursor->worstDuration = std::max(
                _cursor->worstDuration, node->worstDuration);
        _cursor->currentStart = node->currentStart;
        return;
    }

    // No identical child nodes were found.
    _cursor->children.push_back(node);
    node->parent = _cursor;
    _cursor = node;
}

void App::exportGraph() noexcept {
    if(_options.exportGraphTo.length() == 0)
        return;

    Queue<GraphNodePtr> open;
    HashSet<GraphNodePtr> done;
    open.push(_root);

    std::map<GraphNode::Type,Str> typeColor;
    typeColor[GraphNode::Type::START] = "MediumAquamarine";
    typeColor[GraphNode::Type::END] = "MediumAquamarine";
    typeColor[GraphNode::Type::PUSH] = "LightGreen";
    typeColor[GraphNode::Type::EVENT] = "Plum";
    typeColor[GraphNode::Type::BLOCK] = "LightBlue";
    typeColor[GraphNode::Type::SPLIT] = "Yellow";
    typeColor[GraphNode::Type::META] = "Khaki";
    typeColor[GraphNode::Type::EXPECT] = "Aquamarine";
    typeColor[GraphNode::Type::ERROR] = "LightCoral";
    typeColor[GraphNode::Type::ACTION_ERROR] = "Pink";
    typeColor[GraphNode::Type::PLAN_ACCEPTED] = "LightCyan";
    typeColor[GraphNode::Type::PLAN_CHANGED] = "SkyBlue";

    std::stringstream ss;
    ss << "// Generated by LibMozok debugger." << std::endl;
    ss << "// Script: " << _options.scriptFileName << std::endl;
    ss << "digraph mozok {\n" << std::endl;
    while(open.empty() == false) {
        GraphNodePtr node = open.front();
        open.pop();
        done.insert(node);
        for(const auto& ch : node->children)
            open.push(ch);

        Str color = "lightgray";
        if(typeColor.find(node->type) != typeColor.end())
            color = typeColor[node->type];
        int dur = int(node->worstDuration.count());
        int pv = 16 - std::min(16, (dur*16) / _options.maxWaitTime_ms);
        Str p = std::to_string(pv);
        if(pv >= 15) p = "f";
        if(pv == 14) p = "e";
        if(pv == 13) p = "d";
        if(pv == 12) p = "c";
        if(pv == 11) p = "b";
        if(pv == 10) p = "a";
        Str dcol = "#ff" + p+p+p+p;

        // Add new node.
        ss << "\tP" << (void*)(node.get());
        ss << " [\n\t\tshape=plaintext" << std::endl;
        ss << "\t\tlabel=<" << std::endl;
        ss << "\t\t\t<table BORDER='1' CELLBORDER='1' CELLSPACING='0'>\n";
        // Title
        ss << "\t\t\t\t<tr><td COLOR='" << color << "' BGCOLOR='" << color << "'>"
           << "<b>" << node->title << "</b></td>" 
           << "<td BGCOLOR='" << dcol << "' COLOR='" << dcol << "'>" 
           << dur << "<i>ms</i>" << "</td></tr>" << std::endl;
        // Rows
        int rows = 0;
        for(const auto& t : node->text) {
            rows++;
            if((_options.visibilityFlags & int(AppOptions::DETAILS)) == 0)
                if(rows > 2) {
                    ss << "\t\t\t\t<tr><td COLOR='lightgray' COLSPAN='2'>" 
                       << "...</td></tr>" << std::endl;
                    break;
                }
            ss << "\t\t\t\t<tr><td COLOR='lightgray' COLSPAN='2'>" 
               << t << "</td></tr>" << std::endl;
        }
        ss << "\t\t\t</table>" << std::endl;
        ss << "\t\t> ];" << std::endl;

        // Add connection to the parent node.
        if(node->parent != nullptr) {
            ss << "\tP" << (void*)(node->parent.get());
            ss << " -> ";
            ss << "P" << (void*)(node.get());
            ss << ";" << std::endl;
        }
    }
    ss << "\n}" << std::endl;

    std::ofstream file;
    file.open(_options.exportGraphTo, std::ios::out);
    if(file.is_open() == false) {
        _status <<= Result::Error(
                "exportGraph(): Can't write into `" 
                + _options.exportGraphTo + "`!");
        return;
    }

    file << ss.str();
    file.close();
}

// ---------------------------- ACTIONS ------------------------------------ //

bool App::applyNext(QuestRecPtr& rec) noexcept {
    if(rec->lastStatus != QuestStatus::MOZOK_QUEST_STATUS_REACHABLE)
        return false;
    if(rec->nextAction < 0)
        // Waiting for a plan.
        return false;
    if(SIZE_T(rec->nextAction) >= rec->lastPlan_Args.size())
        // Plan was implemented, wait for status change to DONE.
        return false;
    const Str& nextAction = rec->lastPlan_Actions[SIZE_T(rec->nextAction)];
    Server::ActionStatus actionStatus = _currentServer->getActionStatus(
            rec->worldName, nextAction);
    if(actionStatus == Server::ACTION_APPLICABLE) {
        // Action is applicable, push it into a worker thread.
        pushAction(
                false, rec->worldName, nextAction, 
                rec->lastPlan_Args[rec->nextAction], 
                rec->recId);
        rec->nextAction++;
        return true;
    } else if(actionStatus == Server::ACTION_NOT_APPLICABLE) {
        // N/A means that there open subquests.
        // Walk trough all the subquests and try to apply an action there.
        bool allDone = true;
        int doneCount = 0;
        for(auto& sq_rec : rec->subquests) {
            QuestStatus expectedStatus = QuestStatus::MOZOK_QUEST_STATUS_DONE;
            if(sq_rec->expectDone == false)
                expectedStatus = QuestStatus::MOZOK_QUEST_STATUS_UNREACHABLE;
            allDone &= sq_rec->lastStatus == expectedStatus;
            if(allDone) {
                ++doneCount;
                if(doneCount > rec->skippedActions)
                    break;
            } else 
                if(applyNext(sq_rec))
                    return true;
        }
        if(allDone && doneCount > rec->skippedActions) {
            // All subquests are done. 
            // This only means that we can now skip N/A action.
            pushAction(
                true, rec->worldName, nextAction, 
                rec->lastPlan_Args[rec->nextAction],
                rec->recId);
            rec->nextAction++;
            rec->skippedActions++;
            return true;
        }
    } else {
        // This isn't possible, but still.
        _status <<= Result::Error(
                "App::applyNext(): Action `" + nextAction + "` is undefined."
                " Quest = " + qname(rec->worldName, rec->questName) + ".");
        return false;
    }
    return false;
}

// Make sure cmd is a PUSH command!
void App::pushAction(const DebugCmd& cmd, const int data) noexcept {
    StrVec args;
    for(auto it = cmd._args.cbegin() + 2; it != cmd._args.cend(); ++it)
        args.push_back(it->str);
    pushAction(false, cmd._args[0].str, cmd._args[1].str, args, data);
}

void App::pushAction(
        bool isNA,
        const Str& worldName, 
        const Str& actionName, 
        const StrVec& args,
        const int data
        ) noexcept {
    // Info message.
    Str pre = (isNA ? "skip N/A action" : "pushAction");
    if(_options.colorText)
        pre = "\033[92m" + pre + "\033[0m";
    Str text = pre + " [" + worldName + "] " + actionName + "(";
    for(SIZE_T i=0; i<args.size(); ++i)
        text += args[i] + (i != args.size()-1 ? "," : "");
    text += ")";
    infoMsg(text);
    
    recordPush(isNA, worldName, actionName, args, data);

    if(isNA == false)
        _currentServer->pushAction(worldName, actionName, args, data);
    _status <<= onEvent(_onAction, worldName, actionName, args);
}

bool App::applyNextApplicableAction() noexcept {
    for(auto& mq : _mainQuests)
        if(applyNext(mq))
            return true;
    return false;
}

// --------------------------- SIMULATION ---------------------------------- //

App::CheckStatus App::checkQuestExpectations() noexcept {
    for(auto &r : _records) {
        const QuestRecPtr& rec = r.second;
        if(rec->expectDone) {
            if(rec->lastStatus == QuestStatus::MOZOK_QUEST_STATUS_UNREACHABLE) {
                infoMsg("Quest `" + qname(rec->worldName, rec->questName)
                        + "` is unreachable (but expected to be DONE).");
                return STATUS_FAILED;
            }
        } else {
            if(rec->lastStatus == QuestStatus::MOZOK_QUEST_STATUS_DONE) {
                infoMsg("Quest `" + qname(rec->worldName, rec->questName)
                        + "` is done (but expected to be UNREACHABLE).");
                return STATUS_FAILED;
            }
        }
    }
    for(auto &r : _records) {
        const QuestRecPtr& rec = r.second;
        if(rec->expectDone) {
            if(rec->lastStatus != QuestStatus::MOZOK_QUEST_STATUS_DONE)
                return STATUS_WAITING;
        } else {
            if(rec->lastStatus != QuestStatus::MOZOK_QUEST_STATUS_UNREACHABLE)
                return STATUS_WAITING;
        }
    }
    return STATUS_DONE;
}

void App::simulateNext() noexcept {
    const Str pre = (_options.colorText ? "\033[95m" : "");
    const Str post = (_options.colorText ? "\033[0m" : "");
    infoMsg(pre + "=============== NEW TIMELINE ===============" + post);

    // Reset handler sets.
    std::map<EventHandler::Event, HandlerSet*> hmap;
    hmap[EventHandler::ON_SEARCH_LIMIT_REACHED] = &_onSearchLimitReached;
    hmap[EventHandler::ON_NEW_MAIN_QUEST] = &_onNewMainQuest;
    hmap[EventHandler::ON_NEW_MAIN_QUEST] = &_onNewMainQuest;
    hmap[EventHandler::ON_NEW_SUBQUEST] = &_onNewSubQuest;
    hmap[EventHandler::ON_NEW_QUEST_STATUS] = &_onNewQuestStatus;
    hmap[EventHandler::ON_ACTION] = &_onAction;
    hmap[EventHandler::ON_INIT] = &_onInit;
    hmap[EventHandler::ON_PRE] = &_onPre;
    for(auto& h : hmap)
        h.second->clear();
    for(HandlerId i = 0; i < _eventHandlers.size(); ++i) {
        EventHandler::Event e = _eventHandlers[i]._event;
        if(hmap.find(e) != hmap.end())
            hmap[e]->insert(i);
    }

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
    
    recordStart();

    // Activate `onInit` events.
    onEvent(_onInit);

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
        if(applyNextApplicableAction())
            isWaiting = false;

        // Process the messages.
        while(_currentServer->processNextMessage(*this)) {
            // New message has been processed.
            isWaiting = false;
            if(getCurrentStatus().isError())
                break;
            if(_exit)
                break;
        }

        if(getCurrentStatus().isError())
            break;
        if(isWaiting) {
            CheckStatus s = checkQuestExpectations();
            if(s == STATUS_DONE) {
                infoMsg("All quest expectations are met.");
                break;
            } else if(s == STATUS_FAILED) {
                _status <<= Result::Error("Quest expectations failed.");
                break;
            }
        }

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

    if(_status.isError()) {
        recordError();
        errorMsg("Oops! error has been occured during the simulation.");
        infoMsg("Info:\n" + getInfo());
        _callback->onError(this);
    } else 
        recordEnd();

    _mainQuests.clear();
    _allQuests.clear();
    _records.clear();
    _currentServer.reset();
    _activeSplits.clear();
    _pathLog = "";
}

Result App::simulate(AppCallback* callback) noexcept {
    _callback = callback;
    _alternatives.clear();
    _currentPath.clear();
    _alternatives[_currentPath] = 0;
    _donePaths.clear();

    recordReset();

    do {
        simulateNext();
        if(_status.isError())
            break;
        if(_exit)
            break;
        Path p = _currentPath;
        // Delete this alternative when it's a leaf node.
        if(_alternatives[p] == 0) {
            _alternatives.erase(p);
            _donePaths.insert(p);
        }
        // Mark this timeline as completed. 
        // Remove all the parent alternatives without children.
        while(p.size() > 0) {
            p.pop_back();
            _alternatives[p]--;
            if(_alternatives[p] == 0) {
                _alternatives.erase(p);
                _donePaths.insert(p);
            }
        }
        _currentPath.clear();
    } while(_alternatives.empty() == false);

    exportGraph();

    _callback = nullptr;
    return _status;
}

}
}
