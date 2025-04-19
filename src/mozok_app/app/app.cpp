#include "app/app.hpp"
#include "app/block.hpp"
#include "app/callback.hpp"
#include "app/handler.hpp"
#include "app/script.hpp"
#include "app/filesystem.hpp"

#include <libmozok/message_processor.hpp>
#include <libmozok/private_types.hpp>
#include <libmozok/error_utils.hpp>
#include <libmozok/server.hpp>

#include <iostream>
#include <memory>
#include <ostream>
#include <sstream>
#include <chrono>
#include <map>
#include <string>

namespace mozok {
namespace app {

namespace {
Result errNotImplemented(const Str& what) {
    return Result::Error("App::" + what + " is not yet implemented!");
}
inline Str qname(const Str& w, const Str& q) noexcept {
    return w + Str(".") + q;
}
}

QuestRec::QuestRec(
        const Str& world, 
        const Str& quest, 
        const bool isMain
        ) noexcept :
    worldName(world),
    questName(quest),
    isMainQuest(isMain),
    lastStatus(QuestStatus::MOZOK_QUEST_STATUS_UNKNOWN),
    nextAction(-1),
    skippedActions(0)
{ /* empty */ }


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

void App::infoMsg(const Str& text) const noexcept {
    if(_options.verbose) {
        if(_options.colorText)
            std::cout << msg(Str("\033[96mINFO:\033[0m ") + text) << std::endl;
        else
            std::cout << msg(Str("INFO: ") + text) << std::endl;
    }
}

void App::errorMsg(const Str& text) const noexcept {
    if(_options.verbose) {
        if(_options.colorText)
            std::cout << msg(Str("\033[91mERROR:\033[0m ") + text) << std::endl;
        else
            std::cout << msg(Str("ERROR: ") + text) << std::endl;
    }
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
    Str m = "";
    if(cmd._args.size() > 0 && cmd._args.front().type == DebugArg::STR)
        m = cmd._args.front().str;

    const Str pre = (_options.colorText ? "\033[93m" : "");
    const Str post = (_options.colorText ? "\033[0m" : "");

    switch(cmd._cmd) {
        case DebugCmd::PRINT:
            std::cout << msg(pre + "PRINT: " + post + m) << std::endl;
            break;
        case DebugCmd::PAUSE:
            std::cout << msg(pre + "PAUSE: " + post + m) << std::endl;
            _callback->onPause(this);
            break;
        case DebugCmd::EXIT:
            std::cout << msg(pre + "EXIT: " + post + m) << std::endl;
            _exit = true;
            break;
        default:
            return errorNotImplemented(__FILE__, __LINE__, __FUNCTION__);
    }
    return Result::OK();
}

Result App::applyDebugBlock(const DebugBlock& block) noexcept {
    Result res;
    for(const auto& cmd : block._cmds) {
        res <<= applyDebugCmd(cmd);
        if(res.isError())
            break;
    }
    return res;
}

template<typename ...Args>
Result App::onEvent(
        HandlerSet& hset, 
        const Args&... args
        ) noexcept {
    Result res;
    for(auto it = hset.begin(); it != hset.end(); ) {
        const HandlerId hid = *it;
        const auto& h = _eventHandlers[hid];
        //std::cout << "BLOCK: " << h._block._name << std::endl;
        if(match(h._args, args...)) {
            if(h._block._type == DebugBlock::SPLIT) {
                //res <<= applyDebugBlock(h._block);
            } else
                res <<= applyDebugBlock(h._block);

            if(h._block._type != DebugBlock::ALWAYS)
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
        const Result& errorResult
        ) noexcept {
    Str text = "onActionError: [" + worldName + "]";
    text += actionName + "(";
    for(SIZE_T i=0; i<actionArguments.size(); ++i)
            text += actionArguments[i] 
                    + (i != actionArguments.size()-1 ? "," : "");
    text += "). Error result = `" + errorResult.getDescription() + "`";
    errorMsg(text);
    _exit = true;
}

void App::onNewMainQuest(
        const Str& worldName, 
        const Str& questName
        ) noexcept {
    infoMsg("EVENT: onNewMainQuest [" + worldName + "] " + questName);
    QuestRecPtr rec(new QuestRec(worldName, questName, true));
    _mainQuests.push_back(rec);
    _records[qname(worldName, questName)] = rec;
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
    QuestRecPtr rec(new QuestRec(worldName, questName, true));
    _records[qname(worldName, questName)] = rec;
    _records[qname(worldName, parentQuestName)]->subquests.push_back(rec);
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
    infoMsg("EVENT: onNewQuestStatus [" + worldName + "] " 
            + questName + " " + questStatusToStr(questStatus));
    _records[qname(worldName, questName)]->lastStatus = questStatus;
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
        rec->nextAction = 0;
        rec->lastPlan_Actions = actionList;
        rec->lastPlan_Args = actionArgsList;
    }
}

void App::onSearchLimitReached(
        const mozok::Str& worldName,
        const mozok::Str& questName,
        const int /*searchLimitValue*/
        ) noexcept {
    infoMsg("EVENT: onSearchLimitReached [" + worldName + "] " + questName);
    _status <<= onEvent(
            _onSearchLimitReached, worldName, questName);
}
    
void App::onSpaceLimitReached(
        const mozok::Str& worldName,
        const mozok::Str& questName,
        const int /*searchLimitValue*/
        ) noexcept {
    infoMsg("EVENT: onSpaceLimitReached [" + worldName + "] " + questName);
    _status <<= onEvent(
            _onSpaceLimitReached, worldName, questName);
}

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
                rec->lastPlan_Args[rec->nextAction]);
        rec->nextAction++;
        return true;
    } else if(actionStatus == Server::ACTION_NOT_APPLICABLE) {
        // N/A means that there open subquests.
        // Walk trough all the subquests and try to apply an action there.
        bool allDone = true;
        int doneCount = 0;
        for(auto& sq_rec : rec->subquests) {
            allDone &= sq_rec->lastStatus == QuestStatus::MOZOK_QUEST_STATUS_DONE;
            if(allDone)
                ++doneCount;
            else 
            if(applyNext(sq_rec))
                return true;
        }
        if(allDone && doneCount < rec->skippedActions) {
            // All subquests are done. 
            // This only means that we can now skip N/A action.
            pushAction(
                true, rec->worldName, nextAction, 
                rec->lastPlan_Args[rec->nextAction]);
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

void App::pushAction(
        bool isNA,
        const Str& worldName, 
        const Str& actionName, 
        const StrVec& args
        ) noexcept {
    if(_options.verbose) {
        Str pre = (isNA ? "skip N/A action" : "pushAction");
        Str text = pre + " [" + worldName + "] " + actionName + "(";
        for(SIZE_T i=0; i<args.size(); ++i)
            text += args[i] + (i != args.size()-1 ? "," : "");
        text += ")";
        infoMsg(text);
    }
    if(isNA == false)
        _currentServer->pushAction(worldName, actionName, args);
    //
    // TODO: onAction event here
    //
}

bool App::applyNextApplicableAction() noexcept {
    for(auto& mq : _mainQuests)
        if(applyNext(mq))
            return true;
    return false;
}

bool App::isAllQuestsDone() noexcept {
    for(auto &r : _records)
        if(r.second->lastStatus != QuestStatus::MOZOK_QUEST_STATUS_DONE)
            return false;
    infoMsg("All quests are DONE.");
    return true;
}

void App::simulateNext() noexcept {
    // Reset handler sets.
    std::map<EventHandler::Event, HandlerSet*> hmap;
    hmap[EventHandler::ON_SEARCH_LIMIT_REACHED] = &_onSearchLimitReached;
    hmap[EventHandler::ON_NEW_MAIN_QUEST] = &_onNewMainQuest;
    hmap[EventHandler::ON_NEW_MAIN_QUEST] = &_onNewMainQuest;
    hmap[EventHandler::ON_NEW_SUBQUEST] = &_onNewSubQuest;
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

        // Try to process the next message.
        if(_currentServer->processNextMessage(*this)) {
            // New message has been processed.
            isWaiting = false;
            continue;
        }

        if(getCurrentStatus().isError())
            break;
        if(isAllQuestsDone() == true)
            break;

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
    _mainQuests.clear();
    _records.clear();
    _currentServer.reset();
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
