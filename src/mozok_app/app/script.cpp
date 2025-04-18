// ...

#include "app/script.hpp"
#include "app/app.hpp"
#include "app/argument.hpp"
#include "app/block.hpp"
#include "app/command.hpp"
#include "app/handler.hpp"
#include "app/filesystem.hpp"

#include <libmozok/server.hpp>
#include <libmozok/error_utils.hpp>
#include <libmozok/public_types.hpp>
#include <libmozok/result.hpp>
#include <libmozok/script.hpp>

#include <set>

namespace mozok {
namespace app {

namespace {

const Str ON_NEW_MAIN_QUEST = "onNewMainQuest";
const Str ON_NEW_SUBQUEST = "onNewSubQuest";
const Str ON_SEARCH_LIMIT_REACHED = "onSearchLimitReached";
const Str ON_PRE = "onPre";
const Str ON_ACTION = "onAction";
const Str ON_INIT = "onInit";

const Str BLOCK_ACT = "ACT";
const Str BLOCK_SPLIT = "SPLIT";
const Str BLOCK_ALWAYS = "ALWAYS";

const std::set<Str> ALLOWED_BLOCKS = {
        BLOCK_ACT,
        BLOCK_SPLIT,
        BLOCK_ALWAYS };

const Str CMD_PRINT = "print";
const Str CMD_EXIT = "exit";
const Str CMD_PAUSE = "pause";
const Str CMD_EXPECT = "expect";
const Str CMD_APPLY = "apply";

const Str QEVENT_UNREACHABLE = "UNREACHABLE";
const Str QEVENT_GOAL_CHANGE = "GOAL_CHANGE";
const Str QEVENT_SUBQUEST = "SUBQUEST";

const DebugCmd ERROR_CMD = DebugCmd::print("ERROR");


class QuestScriptParser : public QuestScriptParser_Base {
    App* _app;
    Server* _server;
    const bool _deleteServer;

    bool hasSubQuest(const Str& worldName, const Str& subquestName) noexcept {
        return _server->hasSubQuest(worldName, subquestName);
    }
    bool hasMainQuest(const Str& worldName, const Str& mainQuestName) noexcept {
        return _server->hasMainQuest(worldName, mainQuestName);
    }
    bool hasQuest(const Str& world, const Str& quest) noexcept {
        return hasSubQuest(world, quest) || hasMainQuest(world, quest);
    }

protected:

    Result action_with_checks(
            Str& worldName, Str& actionName, StrVec& args) noexcept {
        Result res = action(worldName, actionName, args);
        if(res.isError())
            return res;
        res <<= _server->checkAction(true, worldName, actionName, args);
        return res;
    }


    DebugArg str_arg(Result &res) noexcept {
        if(_src[_pos] == '_') {
            ++_pos;
            ++_col;
            return DebugArg();
        }
        Str str;
        res <<= name(str);
        return DebugArg(str);
    }

    DebugArg num_arg(Result &res) noexcept {
        if(_src[_pos] == '_') {
            ++_pos;
            ++_col;
            return DebugArg();
        }
        int num;
        res <<= pos_int(num);
        return DebugArg(num);
    }

    DebugCmd split(Result &res) noexcept {
        res <<= keyword(BLOCK_SPLIT.c_str());
        res <<= space(1);
        Str splitName;
        res <<= name(splitName);
        res <<= colon_with_spaces();
        res <<= next_line();
        res <<= empty_lines();
        if(res.isError())
            return ERROR_CMD;
        return DebugCmd::split(splitName);
    }

    DebugCmd debug_cmd(Result& res) noexcept {
        Str cmd;
        res <<= name(cmd, LOWER);
        res <<= space(1);
        if(res.isError())
            return ERROR_CMD;

        if(cmd == CMD_EXIT) {
            Str msg;
            res <<= rest(msg);
            return DebugCmd::exit(msg);

        } else if(cmd == CMD_PRINT) {
            Str msg;
            res <<= rest(msg);
            return DebugCmd::print(msg);

        } else if(cmd == CMD_APPLY) {
            Str worldName, actionName;
            StrVec args;
            res <<= action_with_checks(worldName, actionName, args);
            if(res.isError())
                return ERROR_CMD;
            return DebugCmd::apply(worldName, actionName, args);

        } else if(cmd == CMD_EXPECT) {
            Str questEventName, worldName;
            res <<= name(questEventName, UPPER);
            res <<= space(1);
            res <<= world(worldName);
            res <<= space(1);
            if(res.isError())
                return ERROR_CMD;
            if(_server->hasWorld(worldName) == false) {
                res <<= errorWorldDoesntExist(
                        _app->getAppOptions().serverName, worldName);
                return ERROR_CMD;
            }
            if(questEventName == QEVENT_UNREACHABLE) {
                Str questName;
                res <<= name(questName, UPPER);
                res <<= space(0);
                if(res.isError())
                    return ERROR_CMD;
                if(hasQuest(worldName, questName) == false) {
                    res <<= errorUndefinedQuest(
                            worldName, questName);
                    return ERROR_CMD;
                }
                return DebugCmd::expectUnreachable(
                        worldName, questName);

            } else if(questEventName == QEVENT_GOAL_CHANGE) {
                Str questName;
                res <<= name(questName, UPPER);
                res <<= space(1);
                DebugArg from = num_arg(res);
                res <<= space(1);
                DebugArg to = num_arg(res);
                if(res.isError())
                    return ERROR_CMD;
                if(hasQuest(worldName, questName) == false) {
                    res <<= errorUndefinedQuest(
                            worldName, questName);
                    return ERROR_CMD;
                }
                return DebugCmd::expectGoalChange(
                        worldName, questName, from, to);

            } else if(questEventName == QEVENT_SUBQUEST) {
                Str subquestName, parentQuestName;
                res <<= name(subquestName, UPPER);
                res <<= space(1);
                res <<= name(parentQuestName, UPPER);
                res <<= space(1);
                DebugArg goal = num_arg(res);
                res <<= space(0);
                if(res.isError())
                    return ERROR_CMD;
                if(hasSubQuest(worldName, subquestName) == false) {
                    res <<= errorUndefinedSubQuest(worldName, subquestName);
                    return ERROR_CMD;
                }
                if(hasQuest(worldName, parentQuestName) == false) {
                    res <<= errorUndefinedSubQuest(worldName, parentQuestName);
                    return ERROR_CMD;
                }
                return DebugCmd::expectSubquest(
                        worldName, subquestName, parentQuestName, goal);

            } else {
                res <<= errorMsg("Unknown quest event `"+questEventName+"`.");
                return ERROR_CMD;
            }
        }

        // Unknown command.
        res <<= errorMsg("Unknown debug command `" + cmd + "`.");
        return ERROR_CMD;
    }

    DebugBlock block(Result &res) noexcept {
        Str blockType, blockName, tmp;
        bool isSplitBlock = false;
        res <<= name(blockType, UPPER);
        if(res.isError()) {
            Str allowed;
            for(const auto& v : ALLOWED_BLOCKS)
                allowed += v + "; ";
            res <<= errorMsg("Expecting block: " + allowed);
            return DebugBlock::empty();
        }

        if(ALLOWED_BLOCKS.find(blockType) == ALLOWED_BLOCKS.end()) {
            res <<= errorMsg("Unsupported block type `" + blockType + "`.");
            return DebugBlock::empty();
        }

        if(blockType == BLOCK_SPLIT) {
            isSplitBlock = true;
            _pos -= _col;
            _col = 0;
        } else {
            res <<= space(1);
            res <<= name(blockName);
            res <<= colon_with_spaces();
            res <<= next_line();
        }

        if(res.isError())
            return DebugBlock::empty();

        DebugCmdVec cmds;

        while(true) {
            // Check if this is the end of the block.
            if(space(1).isError()) {
                if(isSplitBlock == false)
                    break;
                else {
                    // Check if this is a SPLIT sub-block.
                    if(keyword(BLOCK_SPLIT.c_str()).isError())
                        break;
                    else {
                        _pos -= _col;
                        _col = 0;
                        cmds.push_back(split(res));
                        if(res.isError())
                            return DebugBlock::empty();
                    }
                }
            } else {
                // Parse the next debug command.
                cmds.push_back(debug_cmd(res));
                res <<= space(0);
                res <<= next_line();
                res <<= empty_lines();
                if(res.isError()) {
                    res <<= errorMsg("Invalid debug command. See prev. error.");
                    return DebugBlock::empty();
                }
            }
        }

        if(blockType == BLOCK_ACT)
            return DebugBlock::act(blockName, cmds);
        if(blockType == BLOCK_ALWAYS)
            return DebugBlock::always(blockName, cmds);
        if(blockType == BLOCK_SPLIT)
            return DebugBlock::split(cmds);

        res <<= errorMsg("Unsupported block type `" + blockType + "`");
        return DebugBlock::empty();
    }

    Result onNewMainQuest(const Str& worldName) noexcept {
        Result res;
        Str mainQuestName;

        res <<= space(1);
        res <<= name(mainQuestName, UPPER);

        if(res.isError())
            return res;
        if(hasMainQuest(worldName, mainQuestName) == false)
            return res <<= errorUndefinedMainQuest(worldName, mainQuestName);

        res <<= colon_with_spaces();
        res <<= next_line();
        res <<= empty_lines();
        if(res.isError())
            return res;

        DebugBlock eventBlock = block(res);
        if(res.isError())
            return res;

        EventHandler handler = EventHandler::onNewMainQuest(
                worldName, mainQuestName, eventBlock);
        res <<= _app->addEventHandler(handler);
        return res;

    }


    Result onNewSubQuest(const Str& worldName) noexcept {
        Result res;
        Str subquestName;

        res <<= space(1);
        res <<= name(subquestName, UPPER);
        res <<= space(1);

        if(res.isError())
            return res;
        if(hasSubQuest(worldName, subquestName) == false)
            return res <<= errorUndefinedQuest(worldName, subquestName);

        DebugArg parentQuest = str_arg(res);
        if(res.isError())
            return res;

        if(parentQuest.type != DebugArg::ANY)
            if(hasQuest(worldName, parentQuest.str) == false)
                return res <<= errorUndefinedQuest(worldName, parentQuest.str);

        res <<= space(1);
        DebugArg parentGoal = num_arg(res);
        if(res.isError())
            return res;
        
        res <<= colon_with_spaces();
        res <<= next_line();
        res <<= empty_lines();
        if(res.isError())
            return res;

        DebugBlock eventBlock = block(res);
        if(res.isError())
            return res;

        EventHandler handler = EventHandler::onNewSubQuest(
                worldName, subquestName, parentQuest, parentGoal, eventBlock);
        res <<= _app->addEventHandler(handler);
        return res;

    }

    Result onSearchLimitReached(const Str& worldName) noexcept {
        Result res;
        res <<= space(1);
        DebugArg questName = str_arg(res);
        if(questName.type != DebugArg::ANY)
            if(hasQuest(worldName, questName.str) == false)
                return res <<= errorUndefinedQuest(
                        worldName, questName.str);

        res <<= colon_with_spaces();
        res <<= next_line();
        res <<= empty_lines();
        if(res.isError())
            return res;

        DebugBlock eventBlock = block(res);
        if(res.isError())
            return res;

        EventHandler handler = EventHandler::onSearchLimitReached(
                worldName, questName, eventBlock);
        res <<= _app->addEventHandler(handler);
        return res;
    }

    DebugBlock on_Pre_Action(
            Result& res,
            Str& worldName, 
            Str &actionName, 
            StrVec& args
            ) noexcept {
        res <<= action_with_checks(worldName, actionName, args);
        if(res.isError())
            return DebugBlock::empty();
        res <<= colon_with_spaces();
        res <<= next_line();
        res <<= empty_lines();
        if(res.isError())
            return DebugBlock::empty();
        return block(res);
    }

    Result onInit() noexcept {
        Result res;
        res <<= colon_with_spaces();
        res <<= next_line();
        res <<= empty_lines();
        if(res.isError())
            return res;
        DebugBlock eventBlock = block(res);
        if(res.isError())
            return res;
        EventHandler handler = EventHandler::onInit(eventBlock);
        res <<= _app->addEventHandler(handler);
        return res;
    }

    Result onPre() noexcept {
        Str worldName, actionName;
        StrVec args;
        Result res;
        DebugBlock eventBlock = on_Pre_Action(
                res, worldName, actionName, args);
        if(res.isError())
            return res;
        EventHandler handler = EventHandler::onPre(
                worldName, actionName, args, eventBlock);
        res <<= _app->addEventHandler(handler);
        return res;
    }

    Result onAction() noexcept {
        Str worldName, actionName;
        StrVec args;
        Result res;
        DebugBlock eventBlock = on_Pre_Action(
                res, worldName, actionName, args);
        if(res.isError())
            return res;
        if(_server->getActionStatus(
                worldName, actionName) != Server::ACTION_APPLICABLE)
            return errorMsg(
                    "Action `[" + worldName+"] " + actionName + "` "
                    + "is not applicable");
        EventHandler handler = EventHandler::onAction(
                worldName, actionName, args, eventBlock);
        res <<= _app->addEventHandler(handler);
        return res;

    }

    Result parseDebugSection() noexcept {
        Result res;
        Str event;

        while(name(event, LOWER).isOk()) {
            Str worldName;
            int p, c;

            if(event != ON_INIT) {
                res <<= space(1);
                p = _pos;
                c = _col;
                res <<= world(worldName);
                if(res.isError())
                    return res;
                if(_server->hasWorld(worldName) == false)
                    return res <<= errorWorldDoesntExist(
                            _app->getAppOptions().serverName, worldName);
            }

            if(event == ON_NEW_SUBQUEST) {
                res <<= onNewSubQuest(worldName);
            } else if(event == ON_NEW_MAIN_QUEST) {
                res <<= onNewMainQuest(worldName);
            } else if(event == ON_SEARCH_LIMIT_REACHED) {
                res <<= onSearchLimitReached(worldName);
            } else if(event == ON_INIT) {
                res <<= onInit();
            } else if(event == ON_PRE) {
                _pos = p; _col = c;
                res <<= onPre();
            } else if(event == ON_ACTION) {
                _pos = p; _col = c;
                res <<= onAction();
            } else {
                return res <<= errorMsg("Unknown event `" + event + "`.");
            }

            if(res.isError())
                return res <<= errorMsg("Invalid event. See prev. error.");

            res <<= empty_lines();
        }

        res <<= space(0);
        if(_src[_pos] != '\0')
            return res <<= errorMsg("Parser error.");

        return res;
    }

    /// @brief Call this to parse the script file.
    Result parseFile(bool applyInitAction) noexcept {
        Result res;
        StdFileSystem stdFileSystem;
        res <<= parseHeaderFunc(_server, &stdFileSystem, applyInitAction);
        if(res.isOk())
            res <<= parseDebugSection();
        return res;
    }

    /// @brief Call this to parse and apply one debug command.
    Result parseCommand() noexcept {
        Result res;
        DebugCmd cmd = debug_cmd(res);
        if(res.isError())
            return res;
        return res <<= _app->applyDebugCmd(cmd);
    }

public:
    /// @brief For parsing script files.
    QuestScriptParser(
            const Str& fileName, 
            const Str& script,
            App* app) : 
        QuestScriptParser_Base(fileName, script), 
        _app(app),
        _server(Server::createServer(app->getAppOptions().serverName, _status)),
        _deleteServer(true) {
        _status <<= parseFile(app->getAppOptions().applyInitAction);
    }
    
    /// @brief For parsing and applying one command.
    QuestScriptParser(
            const Str& oneCommand, 
            App* app) :
        QuestScriptParser_Base(oneCommand), 
        _app(app),
        _server(app->getCurrentServer()),
        _deleteServer(false) {
        _status <<= parseCommand();
    }

    virtual ~QuestScriptParser() {
        if(_deleteServer) {
            delete _server;
            _server = nullptr;
        }
    }

    Result getStatus() const {
        return _status;
    }
};

}

QSFParser::QSFParser()
{ /* empty */ }

Result QSFParser::parseAndInit(App *app) noexcept {
    const auto& o = app->getAppOptions();
    QuestScriptParser parser(o.scriptFileName, o.scriptFile, app);
    return parser.getStatus();
}

Result QSFParser::parseAndApplyCmd(const Str &command, App *app) noexcept {
    QuestScriptParser parser("[cmd_command]", command, app);
    return parser.getStatus();
}

}
}
