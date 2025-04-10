// Copyright 2024 Pavlo Savchuk. Subject to the MIT license.

#include "libmozok/action.hpp"
#include "libmozok/public_types.hpp"
#include <cstdio>
#include <libmozok/quest_manager.hpp>
#include <libmozok/private_types.hpp>
#include <libmozok/quest.hpp>
#include <libmozok/state.hpp>
#include <libmozok/statement.hpp>
#include <libmozok/quest_planner.hpp>

#include <limits>
#include <utility>

namespace mozok {

namespace {

struct StateNode;
struct StateNodeCmp;

using StateNodePtr = SharedPtr<StateNode>;
using StateNodeQueue = PriorityQueue<StateNodePtr, StateNodeCmp>;

class QuestPlannerActionsIterator;
class QuestHSPRelaxedActionsIterator;

using ActionTable = Vector<int>;
using DifficultyMap = 
    HashMap<const StatementPtr, int, StatementHash, StatementEqual>;

/// @brief A state node in the state graph.
struct StateNode {
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


struct StateNodeCmp {
    bool operator() (const StateNodePtr& a, const StateNodePtr& b) noexcept {
        return a->fScore > b->fScore;
    }
} const stateNodeCmp;


/// @brief A callback class for the `Quest::iterateOverApplicableActions(...)`.
/// This one is the main iterator, used to find a plan for the initial
/// planning problem.
class QuestPlannerActionsIterator : 
        public QuestApplicableActionsIterator {
    const QuestPtr _quest;
    Vector<StatementVec> &_actionPreBuffers;
    /// @brief A node from which we iterate trough the possible substitutions.
    const StateNodePtr _node;
    StateSet& _knownStates;
    const Goal& _goal;
    StateNodeQueue& _openSet;
    const QuestSettings& _settings;
    ActionTable &_tab;
    DifficultyMap &_difficulties;

    const int INF = std::numeric_limits<int>::max();

    /// @brief Calculates simple but surprisingly effective `h()` value.
    inline int calcSimpleHeuristic(const StatePtr& state) const noexcept {
        int h_simp = 0;
        for(const StatementPtr& goalStatement : _goal)
            if(state->hasSubstate({goalStatement}) == false)
                h_simp += int(goalStatement->getArguments().size()) 
                        + _settings.omega;
        return h_simp;
    }

    inline int calcHSPHeuristic_Fast(
            const StatePtr& state,
            const Goal& goal
            ) noexcept {
        //if(state->hasSubstate(goal))
        //    return 0;

        const Quest::PossibleActionVec actions = _quest->getPossibleActions();
        SIZE_T applied_from = _tab.size();

        for(auto &it : _difficulties)
            it.second = INF;
        for(const auto& statement : state->getStatementSet())
            _difficulties[statement] = 0;

        StatePtr relaxedState = state->duplicate();
        SIZE_T actionCount = _quest->getActions().size();

        while(true) {
            bool modified = false;

            for(SIZE_T i=0; i<applied_from; ) {
                const Quest::ActionWithArgs& aa = actions[_tab[i]];
                // check action preconditions
                StatementVec& stvec = _actionPreBuffers[aa.combinedIndx % actionCount];
                if(aa.action->checkActionPreconditions(
                        aa.arguments, relaxedState, stvec) == false) {
                    ++i;
                    continue;
                }
                // `stvec` now contains `preList`
                const auto addList = aa.action->getAddList().substitute(aa.arguments);
                // Apply only the 'add' part of the action.
                relaxedState->addStatements(addList);
                
                // Calculate the action "difficulty".
                int actionDifficulty = 1;
                for(const auto& precondition : stvec)
                    actionDifficulty += _difficulties[precondition];

                // Mark this action as applied
                std::swap(_tab[i], _tab[--applied_from]);

                // Update the difficulties for the statements added by the action.
                for(const auto& added: addList) {
                    auto it = _difficulties.find(added);
                    if(it == _difficulties.end()) {
                        modified = true;
                        _difficulties[added] = actionDifficulty;
                    } else if(it->second > actionDifficulty) {
                        modified = true;
                        it->second = actionDifficulty;
                    }
                }
            }

            if(modified == false)
                break;

            if(relaxedState->hasSubstate(goal))
               break;
        }

        // Get goal difficulty
        int h = 0;
        for(const auto& goalStatement : goal) {
            const auto it = _difficulties.find(goalStatement);
            if(it == _difficulties.cend())
                return INF;
            h += it->second;
        }
        return h;
    }

public:
    QuestPlannerActionsIterator(
            const QuestPtr& quest,
            Vector<StatementVec> &actionPreBuffers,
            const StateNodePtr& node,
            StateSet& knownStates, 
            const Goal& goal,
            StateNodeQueue& openSet,
            const QuestSettings& settings,
            ActionTable &tab,
            DifficultyMap &difficulties
            ) noexcept :
        _quest(quest),
        _actionPreBuffers(actionPreBuffers),
        _node(node),
        _knownStates(knownStates),
        _goal(goal),
        _openSet(openSet),
        _settings(settings),
        _tab(tab),
        _difficulties(difficulties)
    { /* empty */ }

    bool actionCallback(
            const ActionPtr& action, 
            const ObjectVec& arguments,
            const SIZE_T combinedIndx
            ) noexcept {
        if(_openSet.size() > StateNodeQueue::size_type(_settings.spaceLimit))
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
            // A StateNode with such a state already present in the tree.
            return true;
        
        int h_value = 0;
        switch(_settings.heuristic) {
            case QuestHeuristic::SIMPLE:
                h_value = calcSimpleHeuristic(newState);
                break;
            case QuestHeuristic::HSP:
                h_value = calcHSPHeuristic_Fast(newState, _goal);
                break;
            default:
                break;
        }

        // Goal is unreachable from this state.
        if(h_value == INF)
            return true;

        newNode->gScore = _node->gScore + 1;
        newNode->fScore = newNode->gScore + h_value; 
        
        // Insert the new node into the graph and into the open set.
        _knownStates.insert(newState);
        if(_openSet.size() <= StateNodeQueue::size_type(_settings.spaceLimit))
            _openSet.push(newNode);
        
        return true;
    }
};

} // namespace


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
        const QuestSettings& settings
        ) noexcept {
    const GoalVec& goals = _quest->getQuest()->getGoals();
    QuestPlanPtr lastPlan;
    for(GoalVec::size_type goalIndx = _quest->getLastActiveGoalIndx(); 
            goalIndx < goals.size(); 
            ++goalIndx) {
        lastPlan = findGoalPlan(
                ID(goalIndx), worldName, messageProcessor, settings);
        if(lastPlan->status != MOZOK_QUEST_STATUS_UNREACHABLE)
            break;
    }
    return lastPlan;
}

QuestPlanPtr QuestPlanner::findGoalPlan(
        const ID goalIndx,
        const Str& worldName,
        MessageProcessor& messageProcessor,
        const QuestSettings& settings
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

    // Data tables for HSP heuristic.
    ActionTable tab;
    DifficultyMap difficulties;
    if(settings.heuristic == QuestHeuristic::HSP) {
        const Quest::PossibleActionVec actions = 
                _quest->getQuest()->getPossibleActions();
        tab.resize(actions.size());
        for(SIZE_T i=0; i<actions.size(); ++i)
            tab[i] = int(i);
    }

    while(openSet.size() > 0) {
        ++searchStep;
        const bool isSearchLimitReached = 
                searchStep > settings.searchLimit;
        const bool isSpaceLimitReached = 
                int(openSet.size()) > settings.spaceLimit;
        if(isSearchLimitReached || isSpaceLimitReached) {
            if(isSearchLimitReached)
                messageProcessor.onSearchLimitReached(
                    worldName, _quest->getQuest()->getName(), 
                    settings.searchLimit);
            if(isSpaceLimitReached)
                messageProcessor.onSpaceLimitReached(
                    worldName, _quest->getQuest()->getName(), 
                    settings.spaceLimit);
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

        // Get all neighboring states using an actions iterator.
        QuestPlannerActionsIterator it(
            _quest->getQuest(), _actionPreBuffers, node, knownStates, goal,
            openSet, settings, tab, difficulties);
        _quest->getQuest()->iterateOverApplicableActions(
                node->state, it, _actionPreBuffers);
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



}
