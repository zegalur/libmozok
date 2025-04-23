// Copyright 2025 Pavlo Savchuk. Subject to the MIT license.

#pragma once

#include "app/argument.hpp"
#include <libmozok/mozok.hpp>

namespace mozok {
namespace app {

class DebugCmd;
using DebugCmdVec = Vector<DebugCmd>;

class DebugCmd {

    /// @brief Debug command type.
    enum Cmd {
        SPLIT,  /// @brief Split command. Separates the split sub-blocks.
        EXPECT, /// @brief Expect quest event.
        PUSH,   /// @brief Pushes an action into the action queue.
        PAUSE,  /// @brief Pauses debugger.
        PRINT,  /// @brief Prints a text message.
        EXIT    /// @brief Exit debugger.
    };

    /// @brief What quest event to expect (for `expect` command).
    enum QuestEvent {
        NONE,
        //REACHABLE, /// All activated quests expected to be reachable by default.
        UNREACHABLE, /// Expect quest to be unreachable.
        GOAL_CHANGE, /// Expect a specific goal change event.
        SUBQUEST     /// Expect <quest> be a subquest for  <parent> with <goal>.
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
    Str questEventStr() const;
    const DebugArgs& args() const;

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

    static DebugCmd push(
            const Str& worldName,
            const Str& actionName,
            const StrVec& actionArguments
            ) noexcept;
};




}
}
