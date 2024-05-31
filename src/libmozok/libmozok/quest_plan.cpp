// Copyright 2024 Pavlo Savchuk. Subject to the MIT license.

#include <libmozok/quest_plan.hpp>

namespace mozok {

QuestPlan::QuestPlan(
        const ID _givenSubstate, 
        const StatePtr& _givenState,
        const QuestPtr& _quest,
        const ID _goalIndx,
        const QuestStatus _status,
        const ActionVec& _plan
        ) noexcept :
    givenSubstateId(_givenSubstate),
    givenState(_givenState),
    quest(_quest),
    goalIndx(_goalIndx),
    status(_status),
    plan(_plan)
{ /* empty */ }

}