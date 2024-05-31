// Copyright 2024 Pavlo Savchuk. Subject to the MIT license.

#pragma once

#include <libmozok/public_types.hpp>
#include <libmozok/private_types.hpp>
#include <libmozok/result.hpp>

#include <libmozok/statement.hpp>
#include <libmozok/action.hpp>

namespace mozok {

class Quest;
using QuestPtr = SharedPtr<const Quest>;
using QuestVec = Vector<QuestPtr>;

using Goal = StatementVec;
using GoalVec = Vector<Goal>;


/// @brief A callback class for the `Quest::iterateOverApplicableActions(...)`.
class QuestApplicableActionsIterator {
public:
    virtual ~QuestApplicableActionsIterator() noexcept;

    /// @brief This method will be invoked for the next applicable action.
    /// @param action Action that is applicable.
    /// @param arguments Applicable action arguments.
    /// @return Return `true` if you want to continue the search. 
    ///         Return `false` if you want to stop the search.
    virtual bool actionCallback(
            const ActionPtr& /*action*/, 
            const ObjectVec& /*arguments*/
            ) noexcept;
};


/// @brief Quest contains preconditions, goals, related actions and objects and 
///        sub-quests.
/// A quest outlines player goals and the methods to achieve them. A quest may 
/// have multiple goals, sorted by priority. Only a reachable goal with the will 
/// be designated as the current quest goal by the `QuestManager`. If a goal 
/// becomes unreachable, then the next reachable goal will be selected as a goal.
class Quest {
    /// @brief Quest's unique name.
    const Str _name;

    /// @brief Quest's unique ID.
    const ID _id;

    /// @brief Quest's preconditions.
    const StatementVec _preconditions;

    /// @brief Quest's goal vector.
    const GoalVec _goals;

    /// @brief The list of allowed actions.
    const ActionVec _actions;

    /// @brief The list of the allowed and relevant objects.
    const ObjectVec _objects;

    /// @brief The list of subquests.
    const QuestVec _subquests;

    using ActionArgObjects = Vector<Vector<ObjectVec>>;

    /// @brief [action_index][argument_index] = the list of all allowed objects 
    ///        with the suitable type.
    /// This array of arrays will be used in the `iterateOverApplicableActions`.
    const ActionArgObjects _actionArgObjects;
    ActionArgObjects buildActionArgObjects() const noexcept;

    using IdToActionMap = UnorderedMap<ID, ActionPtr>;
    const IdToActionMap _idToAction;
    IdToActionMap buildIdToActionMap() const noexcept;

    /// @brief The set of permitted action IDs.
    const UnorderedSet<ID> _relevantActions;

    /// @brief The set of permitted object IDs.
    const UnorderedSet<ID> _relevantObjects;

    /// @brief The set of relevant relation IDs.
    const UnorderedSet<ID> _relevantRelations;
    
    UnorderedSet<ID> buildRelevantActions(const ActionVec& actions) const noexcept;
    UnorderedSet<ID> buildRelevantObjects(const ObjectVec& objects) const noexcept;
    UnorderedSet<ID> buildRelevantRelations(
            const ActionVec& actions) const noexcept;
    
    /// @brief Iterates trough all allowed objects for a given allowed action.
    /// @param state The state from which the search occurs.
    /// @param it Callback object.
    /// @param actionPreBuffers Action's pre-buffer (see 
    ///         `QuestPlanner::_actionPreBuffers` for the description).
    /// @param objects Current list of selected allowed objects.
    /// @param objectSet The set of all currently selected allowed objects.
    /// @param actionIndx Action's index in the list of quest's allowed actions.
    /// @param argIndx Action argument index (starting from 0).
    /// @return Returns true if no substitutions were found. Returns false if at 
    ///         some point the callback object halts the search. Otherwise, 
    ///         returns true.
    bool findNextObj(
            const StatePtr& state,
            QuestApplicableActionsIterator& it,
            Vector<StatementVec>& actionPreBuffers,
            ObjectVec &objects,
            ObjectSet &objectSet,
            ObjectVec::size_type actionIndx,
            ObjectVec::size_type argIndx
            ) const noexcept;

public:
    Quest(
        const Str& name, 
        const ID id,
        const StatementVec& preconditions,
        const GoalVec& goals,
        const ActionVec& actions,
        const ObjectVec& objects,
        const QuestVec& subquests
        ) noexcept;

    const Str& getName() const noexcept;
    ID getId() const noexcept;
    const StatementVec& getPreconditions() const noexcept;
    const GoalVec& getGoals() const noexcept;
    const ActionVec& getActions() const noexcept;
    const ActionPtr& getAction(const ID actionId) const noexcept;
    const ObjectVec& getObjects() const noexcept;
    const QuestVec& getSubquests() const noexcept;

    /// @brief Iterates trough the possible applicable actions.
    /// @param state The state from which the search occurs.
    /// @param it Callback object.
    /// @param actionPreBuffers Action's pre-buffer (see 
    ///         `QuestPlanner::_actionPreBuffers` for the description).
    void iterateOverApplicableActions(
            const StatePtr& state,
            QuestApplicableActionsIterator& it,
            Vector<StatementVec>& actionPreBuffers
            ) const noexcept;

    /// @brief Checks if given action is listed as allowed for this quest.
    /// @param actionId Action's unique ID.
    /// @return Returns `true` if action is listed as allowed for this quest.
    bool isActionRelevant(const ID actionId) const noexcept;

    /// @brief Checks if given object is listed as allowed for this quest.
    /// @param objectId Objects's unique ID.
    /// @return Returns `true` if object is listed as allowed for this quest.
    bool isObjectRelevant(const ID objectId) const noexcept;

    /// @brief Checks if given object is listed as allowed for this quest.
    /// @param objectId Objects's unique ID.
    /// @return Returns `true` if object is listed as allowed for this quest.
    bool isRelationRelevant(const ID relationId) const noexcept;

};

}