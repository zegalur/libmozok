// ...

#pragma once

#include "app/argument.hpp"
#include "app/block.hpp"

#include <libmozok/private_types.hpp>
#include <libmozok/mozok.hpp>

namespace mozok {
namespace app {


class EventHandler;
using EventHandlers = Vector<EventHandler>;
using HandlerId = EventHandlers::size_type;
using HandlerIds = Vector<HandlerId>;
using HandlerSet = HashSet<HandlerId>;


class EventHandler {
    enum Event {
        ON_NEW_MAIN_QUEST,
        ON_NEW_SUBQUEST,
        ON_SEARCH_LIMIT_REACHED,
        ON_SPACE_LIMIT_REACHED,
        ON_PRE,
        ON_ACTION,
        ON_INIT
    };

    const Event _event;
    const DebugArgs _args;
    const DebugBlock _block;

    EventHandler(
            const Event event,
            const DebugArgs& args,
            const DebugBlock& block
            ) noexcept;

    friend App;

public:

    static EventHandler onNewMainQuest(
            const Str& worldName,
            const Str& mainQuestName,
            const DebugBlock& block
            ) noexcept;

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
    
    static EventHandler onSpaceLimitReached(
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

    static EventHandler onInit(
            const DebugBlock& block
            ) noexcept;

};

}
}
