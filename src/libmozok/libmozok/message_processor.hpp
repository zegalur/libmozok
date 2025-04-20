// Copyright 2024 Pavlo Savchuk. Subject to the MIT license.

#pragma once

#include <libmozok/public_types.hpp>
#include <libmozok/result.hpp>

namespace mozok {

/// @brief Status of a quest within the quest world.
enum QuestStatus {
    // Quest is inactive.
    MOZOK_QUEST_STATUS_INACTIVE,

    // A goal is achieved.
    MOZOK_QUEST_STATUS_DONE,

    // A goal is reachable (and plan is not empty).
    MOZOK_QUEST_STATUS_REACHABLE, 

    // Goal is not reachable.
    MOZOK_QUEST_STATUS_UNREACHABLE, 

    // Status is unknown.
    MOZOK_QUEST_STATUS_UNKNOWN
};

/// @brief Error code, passed to the `onActionError`.
enum ActionError {
    MOZOK_AE_NO_ERROR,
    MOZOK_AE_UNDEFINED_ACTION,
    MOZOK_AE_ARITY_ERROR,
    MOZOK_AE_UNDEFINED_OBJECT,
    MOZOK_AE_TYPE_ERROR,

    // Preconditions not hold.
    MOZOK_AE_PRECONDITIONS_ERROR,

    // Action is N/A and can't be applied.
    MOZOK_AE_NA_ACTION,

    // Any other action error.
    MOZOK_OTHER_ERROR
};

/// @brief Converts a `QuestStatus` value into the corresponding `Str`.
/// @param questStatus Quest status value.
/// @return Returns the corresponding `Str` value.
Str questStatusToStr(const QuestStatus) noexcept;


/// @brief Enables customized message handling through inheritance.
/// LibMozok guarantee that message order is always logically consistent:
///   -# `onNewMainQuest` or `onNewSubQuest` is always before any other messages 
///      related to that quest (e.g. no `onNewQuestStatus` before `onNewMainQuest`).
///   -# If applied action changes are relevant to a quest, then `onNewQuestState`
///      is always before `onNewQuestPlan`.
///   -# Quests marked as DONE or UNREACHABLE do not send the `onNewQuestState` 
///      message.
///   -# If a new plan changes the quest status, then `onNewQuestStatus` is always
///      before `onNewQuestPlan`.
///   -# `onNewMainQuest` of the parent quest is always before `onNewSubQuest` 
///      of the subquest.
///   -# `onNewSubQuest` of the parent quest is always before `onNewSubQuest` of 
///      the subquest.
///   -# `onNewQuestGoal` is always before the `onNewQuestPlan` 
///      (when goal was changed) but after `onNewQuestStatus`.
class MessageProcessor {
public:
    virtual ~MessageProcessor();

    /// @brief An error occurred during an action application.
    /// @param worldName The name of the world from which this message was sent.
    /// @param actionName Action name.
    /// @param actionArguments Action arguments.
    /// @param errorResult Error result.
    /// @param actionError Action error code.
    /// @param data The data pushed along with this action.
    virtual void onActionError(
        const mozok::Str& worldName, 
        const mozok::Str& actionName,
        const mozok::StrVec& actionArguments,
        const mozok::Result& errorResult,
        const mozok::ActionError actionError,
        const int data
        ) noexcept;

    /// @brief Called when a new main quest was found following an applied action.
    /// @param worldName The name of the world from which this message was sent.
    /// @param questName The name of the new main quest.
    virtual void onNewMainQuest(
        const mozok::Str& worldName, 
        const mozok::Str& questName
        ) noexcept;

    /// @brief Triggered when a new subquest is discovered during quest planning.
    /// @param worldName The name of the world from which this message was sent.
    /// @param subquestName The name of the new subquest.
    /// @param parentQuestName The name of the parent quest.
    /// @param goal The index of the associated parent quest goal.
    virtual void onNewSubQuest(
        const mozok::Str& worldName, 
        const mozok::Str& subquestName,
        const mozok::Str& parentQuestName,
        const int goal
        ) noexcept;
    
    /// @brief Triggered when an applied action changes the state in a way that 
    ///        is relevant to this quest.
    /// @param worldName The name of the world from which this message was sent.
    /// @param questName The name of the relevant quest.
    virtual void onNewQuestState(
        const mozok::Str& worldName, 
        const mozok::Str& questName
        ) noexcept;

    /// @brief Triggered when a quest received a new status.
    /// @param worldName The name of the world from which this message was sent.
    /// @param questName The name of the quest.
    /// @param questStatus Quest's new status.
    virtual void onNewQuestStatus(
        const mozok::Str& worldName, 
        const mozok::Str& questName,
        const mozok::QuestStatus questStatus
        ) noexcept;

    /// @brief Triggered when the active goal of a quest changes.
    ///        Quests that are just activated with the default goal (0) 
    ///        do not trigger this event.
    /// @param worldName The name of the world from which this message was sent.
    /// @param questName The name of the quest.
    /// @param newGoal The new goal index (starting from 0, but not called for
    ///     quests just activated with the default goal (0)).
    /// @param oldGoal The previous goal index (starting from 0).
    virtual void onNewQuestGoal(
        const mozok::Str& worldName,
        const mozok::Str& questName,
        const int newGoal,
        const int oldGoal
        ) noexcept;

    /// @brief A new quest plan has been constructed during quest planning.
    /// @param worldName The name of the world from which this message was sent.
    /// @param questName The name of the quest.
    /// @param actionList The list of action names in the new plan.
    /// @param actionArgsList The list of action arguments in the new plan.
    virtual void onNewQuestPlan(
        const mozok::Str& worldName, 
        const mozok::Str& questName,
        const mozok::StrVec& actionList,
        const mozok::Vector<mozok::StrVec>& actionArgsList
        ) noexcept;

    /// @brief A search limit was reached during a quest planning.
    /// @param worldName The name of the world from which this message was sent.
    /// @param questName The name of the quest.
    /// @param searchLimitValue The search limit value.
    virtual void onSearchLimitReached(
        const mozok::Str& worldName,
        const mozok::Str& questName,
        const int searchLimitValue
        ) noexcept;
    
    /// @brief A space limit was reached during a quest planning.
    /// @param worldName The name of the world from which this message was sent.
    /// @param questName The name of the quest.
    /// @param spaceLimitValue The search limit value.
    virtual void onSpaceLimitReached(
        const mozok::Str& worldName,
        const mozok::Str& questName,
        const int spaceLimitValue
        ) noexcept;

};

}
