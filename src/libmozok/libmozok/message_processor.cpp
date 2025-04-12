// Copyright 2024 Pavlo Savchuk. Subject to the MIT license.

#include <libmozok/message_processor.hpp>

namespace mozok {


Str questStatusToStr(const QuestStatus status) noexcept {
    switch(status) {
        case MOZOK_QUEST_STATUS_INACTIVE:
            return "MOZOK_QUEST_STATUS_INACTIVE";
        case MOZOK_QUEST_STATUS_DONE:
            return "MOZOK_QUEST_STATUS_DONE";
        case MOZOK_QUEST_STATUS_REACHABLE:
            return "MOZOK_QUEST_STATUS_REACHABLE";
        case MOZOK_QUEST_STATUS_UNREACHABLE:
            return "MOZOK_QUEST_STATUS_UNREACHABLE";
        case MOZOK_QUEST_STATUS_UNKNOWN:
            return "MOZOK_QUEST_STATUS_UNKNOWN";
        default:
            return "???";
    }
}


MessageProcessor::~MessageProcessor() = default;

void MessageProcessor::onActionError(
        const Str& /*worldName*/, 
        const Str& /*actionName*/,
        const StrVec& /*actionArguments*/,
        const Result& /*errorResult*/
        ) noexcept 
{ /* empty */ }

void MessageProcessor::onNewMainQuest(
        const Str& /*worldName*/, 
        const Str& /*questName*/
        ) noexcept 
{ /* empty */ }

void MessageProcessor::onNewSubQuest(
        const Str& /*worldName*/, 
        const Str& /*questName*/,
        const Str& /*parentQuestName*/,
        const int /*goal*/
        ) noexcept
{ /* empty */ }

void MessageProcessor::onNewQuestState(
        const mozok::Str& /*worldName*/, 
        const mozok::Str& /*questName*/
        ) noexcept
{ /* empty */ }

void MessageProcessor::onNewQuestStatus(
        const Str& /*worldName*/, 
        const Str& /*questName*/,
        const QuestStatus /*questStatus*/
        ) noexcept
{ /* empty */ }

void MessageProcessor::onNewQuestPlan(
        const Str& /*worldName*/, 
        const Str& /*questName*/,
        const StrVec& /*actionList*/,
        const Vector<StrVec>& /*actionArgsList*/
        ) noexcept
{ /* empty */ }

void MessageProcessor::onSearchLimitReached(
        const mozok::Str& /*worldName*/,
        const mozok::Str& /*questName*/,
        const int /*searchLimitValue*/
        ) noexcept
{ /* empty */ }
    
void MessageProcessor::onSpaceLimitReached(
        const mozok::Str& /*worldName*/,
        const mozok::Str& /*questName*/,
        const int /*searchLimitValue*/
        ) noexcept
{ /* empty */ }

}
