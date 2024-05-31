// Copyright 2024 Pavlo Savchuk. Subject to the MIT license.

#pragma once

#include <libmozok/public_types.hpp>
#include <libmozok/private_types.hpp>

#include <libmozok/object.hpp>
#include <libmozok/relation.hpp>
#include <libmozok/statement.hpp>

namespace mozok {

class Quest;

class State;
using StatePtr = SharedPtr<State>;
using StateVec = Vector<StatePtr>;

struct StateHash {
    std::size_t operator()(const StatePtr& state) const noexcept;
};
struct StateEqual {
    bool operator()(const StatePtr& a, const StatePtr& b) const noexcept;
};

using StateSet = HashSet<StatePtr, StateHash, StateEqual>;


/// @brief A quest state is a collection of statements (facts) that describes 
///        the state of the quest world. 
/// Every fact is a statement, where each argument is a real object (with 
/// non-negative ID) from the quest world. The quest state is essentially a set 
/// of relations between these objects.
class State {
    /// @brief The set of all state statements.
    StatementSet _state;

    /// @brief State's hash value.
    /// The state hash function is "XOR-linear," making it easy and
    /// straightforward to compute a new hash value by knowing which states were 
    /// added and removed from the state.
    std::size_t _hash;

    /// @brief Computes a hash value of the current state. 
    /// @return Returns a recalculated hash value.
    std::size_t computeHash() const noexcept;

public:
    State(const StatementVec& statements) noexcept;

    const StatementSet& getStatementSet() const noexcept;
    std::size_t getHash() const noexcept;

    /// @brief Checks if this state contains a given list of statements.
    /// @param substate A list of statements.
    /// @return Returns `true` if this state contains all statements from the 
    ///         given list.
    bool hasSubstate(const StatementVec& substate) const noexcept;

    /// @brief Modifies the state by adding the provided list of statements. 
    /// Automatically calculates the new hash value.
    /// @param substate A list of statements to be added.
    void addStatements(const StatementVec& substate) noexcept;

    /// @brief Modifies the state by removing the provided list of statements. 
    /// Automatically calculates the new hash value.
    /// @param substate A list of statements to be removed.
    void removeStatements(const StatementVec& substate) noexcept;

    /// @brief Creates a full duplicate state.
    StatePtr duplicate() const noexcept;

    /// @brief Creates a duplicate substate containing only statements relevant 
    ///        to the given quest.
    /// @param quest A quest.
    /// @return Returns a duplicate substate.
    StatePtr duplicate(const Quest& quest) const noexcept;
};

}