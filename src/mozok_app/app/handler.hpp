// ...

#pragma once

#include "app/argument.hpp"
#include "app/block.hpp"

#include <libmozok/mozok.hpp>

namespace mozok {
namespace app {

class EventHandler {
    enum Event {
        ON_NEW_SUBQUEST,
        ON_SEARCH_LIMIT_REACHED,
        ON_PRE,
        ON_ACTION
    };

    const Event _event;
    const DebugArgs _args;
    const DebugBlock _block;

    EventHandler(
            const Event event,
            const DebugArgs& args,
            const DebugBlock& block
            ) noexcept;

public:
    static EventHandler onNewSubQuest(
            const Str& worldName,
            const Str& subQuestName,
            const DebugArg& parentQuestName,
            const DebugArg& parentGoal,
            const DebugBlock& block
            ) noexcept;

    static EventHandler onSearchLimitReached(
            const Str& worldName,
            const DebugArg& questName, // can be empty
            const DebugBlock& block
            ) noexcept;

    static EventHandler onPre(
            const Str& worldName,
            const Str& actionName,
            const StrVec& arguments,
            const DebugBlock& block
            ) noexcept;
    
    static EventHandler onAction(
            const Str& worldName,
            const Str& actionName,
            const StrVec& arguments,
            const DebugBlock& block
            ) noexcept;

};

}
}
