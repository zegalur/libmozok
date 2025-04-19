// ...

#pragma once

#include "app/argument.hpp"

#include <libmozok/mozok.hpp>

namespace mozok {
namespace app {

class DebugCmd;
using DebugCmdVec = Vector<DebugCmd>;

class DebugCmd {
    enum Cmd {
        SPLIT, // split command
        EXPECT, // expect quest event
        APPLY, // apply action
        PAUSE, // pause debugger
        PRINT, // print message
        EXIT // exit debugger
    };
    enum QuestEvent {
        NONE, 
        //REACHABLE, // all activated quests expected to be reachable
        UNREACHABLE, // expect quest to be unreachable
        GOAL_CHANGE, // ...
        SUBQUEST // expect <quest> be a subquest for <parent> with <goal>
    };

    const Cmd _cmd;
    const QuestEvent _questEvent;
    const DebugArgs _args;

    DebugCmd(
            const Cmd cmd, 
            const QuestEvent questEvent, 
            const DebugArgs& args
            ) noexcept;

    friend class App;
    friend class DebugBlock;

public:
    static DebugCmd split(const Str& splitName) noexcept;
    static DebugCmd pause(const Str& message) noexcept;
    static DebugCmd print(const Str& message) noexcept;
    static DebugCmd exit(const Str& message) noexcept;

    static DebugCmd expectUnreachable(
            const Str& worldName,
            const Str& questName
            ) noexcept;
    
    static DebugCmd expectGoalChange(
            const Str& worldName,
            const Str& questName,
            const DebugArg from,
            const DebugArg to
            ) noexcept;

    static DebugCmd expectSubquest(
            const Str& worldName,
            const Str& subquestName,
            const Str& parentQuestName,
            const DebugArg parentGoal
            ) noexcept;


    static DebugCmd apply(
            const Str& worldName,
            const Str& actionName,
            const StrVec& actionArguments
            ) noexcept;
};




}
}
