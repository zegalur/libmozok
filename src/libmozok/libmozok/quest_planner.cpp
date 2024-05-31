// Copyright 2024 Pavlo Savchuk. Subject to the MIT license.

#include <libmozok/quest_planner.hpp>

namespace mozok {


/// @brief A state node in the state graph.
struct QuestPlanner::StateNode {
    /// @brief Node state.
    const StatePtr state;

    /// @brief Preceding state on the cheapest path from the initial state 
    ///     to this state.
    const StateNodePtr preceding;

    /// @brief Action that changes state from the preceding to current state.
    const ActionPtr action;

    /// @brief Action arguments.
    ObjectVec arguments;

    /// @brief Cheapest known length from the initial state.
    int gScore;

	/// @brief Best guess of shortest length from the initial state 
    ///     (f(n) = g(n) + h(n)).
	int fScore;

    StateNode(
            const StatePtr _state, 
            const StateNodePtr _preceding,
            const ActionPtr _action
            ) noexcept :
        state(_state),
        preceding(_preceding),
        action(_action),
        gScore(0),
        fScore(0)
    { /* empty */ }
};


struct QuestPlanner::StateNodeCmp {
    bool operator() (const StateNodePtr& a, const StateNodePtr& b) noexcept {
        return a->fScore > b->fScore;
    }
} const stateNodeCmp;


QuestPlanner::QuestPlanner(
        const ID givenSubstateId, 
        const StatePtr& givenState,
        const QuestManagerPtr& quest
        ) noexcept :
    _givenSubstateId(givenSubstateId),
    _givenState(givenState->duplicate()),
    _quest(quest) { 
    createActionPreBuffers();
}

void QuestPlanner::createActionPreBuffers() noexcept {
    _actionPreBuffers.reserve(_quest->getQuest()->getActions().size());
    for(const ActionPtr& action : _quest->getQuest()->getActions()) {
        const RelationList& pre = action->getPreconditions();
        _actionPreBuffers.push_back(pre.substitute(pre.getArguments()));
    }
}

ID QuestPlanner::getGivenSubstateId() const noexcept {
    return _givenSubstateId;
}

const QuestManagerPtr& QuestPlanner::getQuest() const noexcept {
    return _quest;
}

QuestPlanPtr QuestPlanner::findQuestPlan(
        const Str& worldName,
        MessageProcessor& messageProcessor,
        const int searchLimit,
        const int spaceLimit,
        const int omega
        ) noexcept {
    const GoalVec& goals = _quest->getQuest()->getGoals();
    QuestPlanPtr lastPlan;
    for(GoalVec::size_type goalIndx = _quest->getLastActiveGoalIndx(); 
            goalIndx < goals.size(); 
            ++goalIndx) {
        lastPlan = findGoalPlan(
                ID(goalIndx), worldName, messageProcessor, 
                searchLimit, spaceLimit, omega);
        if(lastPlan->status != MOZOK_QUEST_STATUS_UNREACHABLE)
            break;
    }
    return lastPlan;
}

QuestPlanPtr QuestPlanner::findGoalPlan(
        const ID goalIndx,
        const Str& worldName,
        MessageProcessor& messageProcessor,
        const int searchLimit,
        const int spaceLimit,
        const int omega
        ) noexcept {
    const Goal& goal = _quest->getQuest()->getGoals().at(goalIndx);
    if(_givenState->hasSubstate(goal))
        // Quest is already done.
        return makeShared<QuestPlan>(
                _givenSubstateId, _givenState, _quest->getQuest(), goalIndx, 
                MOZOK_QUEST_STATUS_DONE, ActionVec());
    
    StateNodePtr initialStateNode = makeShared<StateNode>(
            _givenState, StateNodePtr(nullptr), ActionPtr(nullptr));
    
    // All discovered states so far.
    StateSet knownStates;

    // States that must be investigated next.
    // Nodes with lower f-score have higher priority.
    StateNodeQueue openSet(stateNodeCmp);
    openSet.push(initialStateNode);

    StateNodePtr finalNode(nullptr);
    int searchStep = 0;

    while(openSet.size() > 0) {
        ++searchStep;
        const bool isSearchLimitReached = searchStep > searchLimit;
        const bool isSpaceLimitReached = int(openSet.size()) > spaceLimit;
        if(isSearchLimitReached || isSpaceLimitReached) {
            if(isSearchLimitReached)
                messageProcessor.onSearchLimitReached(
                    worldName, _quest->getQuest()->getName(), searchLimit);
            if(isSpaceLimitReached)
                messageProcessor.onSpaceLimitReached(
                    worldName, _quest->getQuest()->getName(), spaceLimit);
            // We reach the search limit.
            return makeShared<QuestPlan>(
                    _givenSubstateId, _givenState, _quest->getQuest(), goalIndx, 
                    MOZOK_QUEST_STATUS_UNKNOWN, ActionVec());
        }
        
        // Pop next open node with the smallest f-score.
        StateNodePtr node = openSet.top();
        openSet.pop();

        // Check if node contains all the conditions from the goal.
        if(node->state->hasSubstate(goal)) {
            // We have found the optimal plan.
            finalNode = node;
            break;
        }

        // Get all neighboring states.
        findSubstitutions(node, knownStates, goal, openSet, spaceLimit, omega);
    }

    if(finalNode.get() == nullptr)
        // Goal is unreachable.
        return makeShared<QuestPlan>(
                    _givenSubstateId, _givenState, _quest->getQuest(), goalIndx, 
                    MOZOK_QUEST_STATUS_UNREACHABLE, ActionVec());
    
    // At this point quest goal is reachable.

    // Build quest plan.
    ActionVec plan(finalNode->gScore, ActionPtr(nullptr));
    while(finalNode.get() != nullptr) {
        if(finalNode->action.get() != nullptr)
            plan[finalNode->gScore - 1] = finalNode->action;
        finalNode = finalNode->preceding;
    }
    return makeShared<QuestPlan>(
            _givenSubstateId, _givenState, _quest->getQuest(), goalIndx, 
            MOZOK_QUEST_STATUS_REACHABLE, plan);
}

/// @brief A callback class for the `Quest::iterateOverApplicableActions(...)`.
class QuestPlanner::QuestPlannerActionsIterator : 
        public QuestApplicableActionsIterator {
    /// @brief A node from which we iterate trough the possible substitutions.
    const StateNodePtr _node;
    StateSet& _knownStates;
    const Goal& _goal;
    StateNodeQueue& _openSet;
    int _spaceLimit;
    int _omega;

public:
    QuestPlannerActionsIterator(
            const StateNodePtr& node,
            StateSet& knownStates, 
            const Goal& goal,
            StateNodeQueue& openSet,
            const int spaceLimit,
            const int omega
            ) noexcept :
        _node(node),
        _knownStates(knownStates),
        _goal(goal),
        _openSet(openSet),
        _spaceLimit(spaceLimit),
        _omega(omega)
    { /*empty*/ }

    bool actionCallback(
            const ActionPtr& action, 
            const ObjectVec& arguments
            ) noexcept {
        if(_openSet.size() > StateNodeQueue::size_type(_spaceLimit))
            return false;
        
        StatePtr newState = _node->state->duplicate();
        
        // We can apply the action unsafely because the arguments were selected
        // in such a way that they are fully compatible with the action and with
        // the state.
        action->applyActionUnsafe(arguments, newState); 
        
        // Save the resulting state into a new node.
        StatementVec emptySVec;
        ActionPtr nodeAction = makeShared<Action>(
                action->getName(), action->getId(), action->isNotApplicable(), 
                arguments, emptySVec, emptySVec, emptySVec);
        StateNodePtr newNode = makeShared<StateNode>(newState, _node, nodeAction);

        if(_knownStates.find(newState) != _knownStates.end())
            // A StateNode with such a state already present in the three.
            return true;
        
        // Simple heuristic.
        int h_simp = 0;
        for(const StatementPtr& goalStatement : _goal)
            if(newState->hasSubstate({goalStatement}) == false)
                h_simp += int(goalStatement->getArguments().size()) + _omega;

        newNode->gScore = _node->gScore + 1;
        newNode->fScore = newNode->gScore + h_simp;
        
        // Insert the new node into the graph and into the open set.
        _knownStates.insert(newState);
        if(_openSet.size() <= StateNodeQueue::size_type(_spaceLimit))
            _openSet.push(newNode);
        
        return true;
    }
};

void QuestPlanner::findSubstitutions(
            const StateNodePtr& node, 
            StateSet& knownStates,
            const Goal& goal,
            StateNodeQueue& openSet,
            const int spaceLimit,
            const int omega
            ) noexcept {
    QuestPlannerActionsIterator it(
        node, knownStates, goal, openSet, spaceLimit, omega);
    _quest->getQuest()->iterateOverApplicableActions(
            node->state, it, _actionPreBuffers);
}


}