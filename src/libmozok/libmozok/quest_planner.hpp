// Copyright 2024 Pavlo Savchuk. Subject to the MIT license.

#pragma once

#include <libmozok/public_types.hpp>
#include <libmozok/message_processor.hpp>
#include <libmozok/private_types.hpp>
#include <libmozok/state.hpp>
#include <libmozok/quest.hpp>
#include <libmozok/world.hpp>

#include <libmozok/quest_plan.hpp>
#include <libmozok/quest_manager.hpp>

namespace mozok {

/// @brief Quest planner performs planning for a given quest.
/// A plan is a list of proper actions that leads to the quest completion.
class QuestPlanner {
    /// @brief Quest's substate ID of a given state. 
    const ID _givenSubstateId;

    /// @brief The state from which the planner will attempt to find a plan.
    const StatePtr _givenState;

    /// @brief Quest manager of the quest.
    const QuestManagerPtr _quest;

    /// @brief This array is used for the optimization.
    /// It contains `StatementVec`s that contain identical relations and 
    /// constant arguments as those specified in the action's preconditions.
    /// Given in the order from the quest definition.
    Vector<StatementVec> _actionPreBuffers;

    /// @brief Creates `_actionPreBuffers` vector.
    void createActionPreBuffers() noexcept;

    struct StateNode;
    struct StateNodeCmp;
    class QuestPlannerActionsIterator;
    using StateNodePtr = SharedPtr<StateNode>;
    using StateNodeQueue = PriorityQueue<StateNodePtr, StateNodeCmp>;

    /// @brief Finds a plan for a given goal.
    /// @param goalIndx Goal index.
    /// @param worldName Quest's world name.
    /// @param messageProcessor A message processor.
    /// @return Returns a plan for a given goal.
    QuestPlanPtr findGoalPlan(
            const ID goalIndx, 
            const Str& worldName,
            MessageProcessor& messageProcessor,
            const int searchLimit,
            const int spaceLimit,
            const int omega
            ) noexcept;

    /// @brief Finds all possible next actions for a given node of a state graph.
    /// @param node A node in a state graph.
    /// @param knownStates The set of all known to this point states.
    /// @param goal The goal statements.
    /// @param openSet The queue of open (unprocessed) nodes in the state graph.
    /// @param spaceLimit Specifies a space limit.
    void findSubstitutions(
            const StateNodePtr& node, 
            StateSet& knownStates,
            const Goal& goal,
            StateNodeQueue& openSet,
            const int spaceLimit,
            const int omega
            ) noexcept;

public:
    /// @brief Creates a quest planner.
    /// @param givenSubstateId Quest's substate ID of a given state. 
    /// @param givenState The state from which the it will attempt to find a plan.
    /// @param quest Quest manager of the quest.
    QuestPlanner(
        const ID givenSubstateId, 
        const StatePtr& givenState, 
        const QuestManagerPtr& quest
        ) noexcept;

    ID getGivenSubstateId() const noexcept;
    const QuestManagerPtr& getQuest() const noexcept;

    /// @brief Performs planning.
    /// @param worldName Quest's world name.
    /// @param messageProcessor A message processor.
    /// @return Returns a quest plan.
    QuestPlanPtr findQuestPlan(
        const Str& worldName,
        MessageProcessor& messageProcessor,
        const int searchLimit,
        const int spaceLimit,
        const int omega
        ) noexcept;

};

}