// ...

#include "app/handler.hpp"
#include "app/argument.hpp"

namespace mozok {
namespace app {

EventHandler::EventHandler(
        const Event event,
        const DebugArgs& args,
        const DebugBlock& block
        ) noexcept :
    _event(event),
    _args(args),
    _block(block)
{ /* empty */ }

EventHandler EventHandler::onNewMainQuest(
        const Str& worldName,
        const Str& mainQuestName,
        const DebugBlock& block
        ) noexcept {
    return EventHandler(
            ON_NEW_MAIN_QUEST, 
            {worldName, mainQuestName}, 
            block);
}

EventHandler EventHandler::onNewSubQuest(
        const Str& worldName,
        const Str& subQuestName,
        const DebugArg& parentQuestName,
        const DebugArg& parentGoal,
        const DebugBlock& block
        ) noexcept {
    return EventHandler(
            ON_NEW_SUBQUEST, 
            {worldName, subQuestName, parentQuestName, parentGoal}, 
            block);
}

EventHandler EventHandler::onNewQuestStatus(
        const Str& worldName,
        const Str& questName,
        const Str& newStatus,
        const DebugBlock& block
        ) noexcept {
    return EventHandler(
            ON_NEW_QUEST_STATUS,
            {worldName, questName, newStatus},
            block);
}

EventHandler EventHandler::onSearchLimitReached(
        const Str& worldName,
        const DebugArg& questName, // can be empty
        const DebugBlock& block
        ) noexcept {
    return EventHandler(
            ON_SEARCH_LIMIT_REACHED, 
            {worldName, questName}, block);
}

EventHandler EventHandler::onSpaceLimitReached(
        const Str& worldName,
        const DebugArg& questName, // can be empty
        const DebugBlock& block
        ) noexcept {
    return EventHandler(
            ON_SPACE_LIMIT_REACHED, 
            {worldName, questName}, block);
}

EventHandler EventHandler::onPre(
        const Str& worldName,
        const Str& actionName,
        const StrVec& arguments,
        const DebugBlock& block
        ) noexcept {
    DebugArgs args;
    args.push_back(worldName);
    args.push_back(actionName);
    for(const auto& obj : arguments)
        args.push_back(obj);
    return EventHandler(ON_PRE, args, block);

}
    
EventHandler EventHandler::onAction(
        const Str& worldName,
        const Str& actionName,
        const StrVec& arguments,
        const DebugBlock& block
        ) noexcept {
    DebugArgs args;
    args.push_back(worldName);
    args.push_back(actionName);
    for(const auto& obj : arguments)
        args.push_back(obj);
    return EventHandler(ON_ACTION, args, block);
}

EventHandler EventHandler::onInit(
        const DebugBlock& block
        ) noexcept {
    return EventHandler(ON_INIT, {}, block);
}

}
}
