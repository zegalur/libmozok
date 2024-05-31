// Copyright 2024 Pavlo Savchuk. Subject to the MIT license.

#pragma once

#include <libmozok/public_types.hpp>
#include <libmozok/message_processor.hpp>

#include <libmozok/private_types.hpp>
#include <libmozok/state.hpp>
#include <libmozok/quest.hpp>

namespace mozok {

struct QuestPlan;
using QuestPlanPtr = SharedPtr<const QuestPlan>;

/// @brief Quest plan is a list of appropriate actions that lead to a quest goal.
/// The `plan` is not empty only when `status` is `MOZOK_QUEST_STATUS_REACHABLE`.
struct QuestPlan {
    /// @brief The ID of the substate for which the quest plan was found.
    const ID givenSubstateId;

    /// @brief The state for which the quest plan was found.
    const StatePtr givenState;

    /// @brief The quest for which the quest plan was found.
    const QuestPtr quest;

    /// @brief The quest goal for which the quest plan was found.
    const ID goalIndx;

    /// @brief Quest plan status.
    QuestStatus status;

    // Plan actions doesn't contain action's pre, add and rem statements.
    const ActionVec plan;

    QuestPlan(
        const ID _givenStateId, 
        const StatePtr& _givenSubstate,
        const QuestPtr& _quest,
        const ID _goalIndx,
        const QuestStatus _status,
        const ActionVec& _plan 
        ) noexcept;
};

}