// Copyright 2024 Pavlo Savchuk. Subject to the MIT license.

#pragma once

#include <libmozok/public_types.hpp>
#include <libmozok/private_types.hpp>
#include <libmozok/result.hpp>

#include <libmozok/object.hpp>
#include <libmozok/relation_list.hpp>
#include <libmozok/statement.hpp>
#include <libmozok/state.hpp>
#include <libmozok/message_processor.hpp>

namespace mozok {

class Action;
using ActionPtr = SharedPtr<const Action>;
using ActionVec = Vector<ActionPtr>;
using ActionSet = HashSet<ActionPtr>;

/// @brief Through actions, player alter the state and progress towards quest goals.
/// Each action comprises a name, arguments, preconditions, and effects. 
/// Effects are statements that will be removed and statements that will be 
/// added to the current state. Players can only modify the state of the world 
/// by applying actions. An action first removes and only then adds statements.
class Action {
    /// @brief Action's unique name.
    const Str _name;

    /// @brief Action's unique ID.
    const ID _id;

    /// @brief Is this action not applicable. Not applicable actions cannot be 
    ///        applied with 'World::applyAction()' method. Only N/A actions can
    ///        trigger subquests.
    const bool _isNotApplicable;

    /// @brief The list of action arguments.
    /// n-th argument (starting from 1) has (-n) ID.
    const ObjectVec _arguments;

    /// @brief Action preconditions. 
    /// An action can only be applied when its preconditions are met.
    const RelationList _pre;

    /// @brief Statements that will be removed from the state by this action.
    const RelationList _rem;

    /// @brief Statements that will be added to the state by this action.
    const RelationList _add;
    
    /// @brief Global actions refer to objects other than action arguments.
    /// Global actions have global effects. Local actions only refer to their 
    /// arguments. Only local actions can be listed as quest actions.
    const bool _isGlobal;

    /// @brief Calculates the locality of the action.
    /// @return Returns `true` if action is global, `false` if action is local.
    bool calculateLocality() const noexcept;

public:
    /// @brief Initializes a new action object.
    /// @param name The name of the action in the quest world.
    /// @param id Action's unique ID.
    /// @param isNotApplicable Is this action not applicable.
    /// @param arguments Action arguments with negative IDs.
    /// @param pre Action preconditions. 
    /// @param rem Statements that will be removed by this action.
    /// @param add Statements that will be added by this action.
    Action(
        const Str& name, 
        const ID id, 
        const bool isNotApplicable,
        const ObjectVec arguments,
        const StatementVec pre,
        const StatementVec rem,
        const StatementVec add
        ) noexcept;
    
    const Str& getName() const noexcept;
    ID getId() const noexcept;
    bool isNotApplicable() const noexcept;
    const ObjectVec& getArguments() const noexcept;

    const RelationList& getPreconditions() const noexcept;
    const RelationList& getRemList() const noexcept;
    const RelationList& getAddList() const noexcept;

    /// @brief Global actions refer to objects other than action arguments.
    /// Global actions have global effects. Local actions only refer to their 
    /// arguments. Only local actions can be listed as quest actions.
    bool isGlobal() const noexcept;

    /// @brief Evaluates the applicability with the given arguments and state.
    /// @param doNotCheckPreconditions if `true` - skips preconditions check.
    /// @param arguments Argument objects.
    /// @param state A state.
    /// @param actionError Outputs the action error code into this variable.
    /// @return Returns `Result::OK()` when the action can be applied.
    Result evaluateActionApplicability(
            const bool doNotCheckPreconditions,
            const ObjectVec& arguments, 
            const StatePtr& state,
            ActionError &actionError
            ) const noexcept;

    /// @brief Applies the action to the state.
    /// @param arguments Argument objects.
    /// @param state A state that will be modified by the action.
    /// @param actionError Outputs the action error code into this variable.
    /// @return Returns `Result::OK()` when the action was successfully applied.
    Result applyAction(
            const ObjectVec& arguments, 
            StatePtr& state,
            ActionError &actionError
            ) const noexcept;
    
    /// @brief Applies the action to the state. It's crucial that the argument 
    /// vector aligns perfectly with the action, as this method lacks the checks 
    /// performed by `applyAction` and doesn't provide feedback on the operation's 
    /// success. Using incompatible arguments may break the quest universe or 
    /// result in a critical error.
    /// @param arguments Argument objects that are fully compatible with the action.
    /// @param state A state that will be modified by the action.
    void applyActionUnsafe(
            const ObjectVec& arguments, 
            StatePtr& state
            ) const noexcept;
    
    /// @brief Optimized version of the `evaluateActionApplicability`.
    /// Checks if a given state includes the action's preconditions. Arguments
    /// must be fully compatible with the action. Using incompatible arguments 
    /// may break the quest universe.
    /// @param arguments Argument objects that are fully compatible with the action.
    /// @param state State undergoing the check.
    /// @param preBuffer A `StatementVec` containing identical relations and 
    /// constant arguments as those specified in the action's preconditions.
    /// @return Returns `true` when the given state includes the preconditions.
    bool checkActionPreconditions(
            const ObjectVec& arguments, 
            const StatePtr& state,
            StatementVec& preBuffer
            ) const noexcept;

};

}
