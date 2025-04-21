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

Str actionErrorToStr(const ActionError code) noexcept {
    switch(code) {
        case MOZOK_AE_NO_ERROR:
            return "MOZOK_AE_NO_ERROR";
        case MOZOK_AE_UNDEFINED_ACTION:
            return "MOZOK_AE_UNDEFINED_ACTION";
        case MOZOK_AE_ARITY_ERROR:
            return "MOZOK_AE_ARITY_ERROR";
        case MOZOK_AE_UNDEFINED_OBJECT:
            return "MOZOK_AE_UNDEFINED_OBJECT";
        case MOZOK_AE_TYPE_ERROR:
            return "MOZOK_AE_TYPE_ERROR";
        case MOZOK_AE_PRECONDITIONS_ERROR:
            return "MOZOK_AE_PRECONDITIONS_ERROR";
        case MOZOK_AE_NA_ACTION:
            return "MOZOK_AE_NA_ACTION";
        case MOZOK_OTHER_ERROR:
            return "MOZOK_OTHER_ERROR";
        default:
            return "???";
    }
}

MessageProcessor::~MessageProcessor() = default;

void MessageProcessor::onActionError(
        const Str& /*worldName*/, 
        const Str& /*actionName*/,
        const StrVec& /*actionArguments*/,
        const Result& /*errorResult*/,
        const ActionError /*actionError*/,
        const int /*data*/
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

void MessageProcessor::onNewQuestGoal(
        const Str& /*worldName*/,
        const Str& /*questName*/,
        const int /*newGoal*/,
        const int /*oldGoal*/
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
