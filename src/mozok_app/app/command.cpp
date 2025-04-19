// ...

#include "app/command.hpp"
#include "app/argument.hpp"

namespace mozok {
namespace app {

DebugCmd::DebugCmd(
        const DebugCmd::Cmd cmd, 
        const DebugCmd::QuestEvent questEvent, 
        const DebugArgs& args
        ) noexcept :
    _cmd(cmd),
    _questEvent(questEvent),
    _args(args)
{ /* empty */ }

DebugCmd DebugCmd::split(const Str& splitName) noexcept {
    return DebugCmd(Cmd::SPLIT, QuestEvent::NONE, {DebugArg(splitName)});
}

DebugCmd DebugCmd::pause(const Str& message) noexcept {
    return DebugCmd(Cmd::PAUSE, QuestEvent::NONE, {DebugArg(message)});
}

DebugCmd DebugCmd::print(const Str& message) noexcept {
    return DebugCmd(Cmd::PRINT, QuestEvent::NONE, {DebugArg(message)});
}

DebugCmd DebugCmd::exit(const Str& message) noexcept {
    return DebugCmd(Cmd::EXIT, QuestEvent::NONE, {DebugArg(message)});
}

DebugCmd DebugCmd::expectUnreachable(
        const Str& worldName,
        const Str& questName
        ) noexcept {
    return DebugCmd(Cmd::EXPECT, QuestEvent::UNREACHABLE, 
            { worldName, questName });
}

DebugCmd DebugCmd::expectGoalChange(
        const Str& worldName,
        const Str& questName,
        const DebugArg from,
        const DebugArg to
        ) noexcept {
    return DebugCmd(Cmd::EXPECT, QuestEvent::GOAL_CHANGE, 
            { worldName, questName, from, to });
}

DebugCmd DebugCmd::expectSubquest(
        const Str& worldName,
        const Str& subquestName,
        const Str& parentQuestName,
        const DebugArg parentGoal
        ) noexcept {
    return DebugCmd(Cmd::EXPECT, QuestEvent::SUBQUEST, 
            { worldName, subquestName, parentQuestName, parentGoal });

}

DebugCmd DebugCmd::apply(
        const Str& worldName,
        const Str& actionName,
        const StrVec& actionArguments
        ) noexcept {
    DebugArgs args;
    args.push_back(worldName);
    args.push_back(actionName);
    for(const auto& objName : actionArguments)
        args.push_back(objName);
    return DebugCmd(Cmd::APPLY, QuestEvent::NONE, args);
}


}
}
