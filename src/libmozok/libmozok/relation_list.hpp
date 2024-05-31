// Copyright 2024 Pavlo Savchuk. Subject to the MIT license.

#pragma once

#include <libmozok/public_types.hpp>
#include <libmozok/private_types.hpp>
#include <libmozok/result.hpp>

#include <libmozok/object.hpp>
#include <libmozok/statement.hpp>

namespace mozok {

class RelationList;
using RelationListPtr = SharedPtr<const RelationList>;
using RelationListVec = Vector<RelationListPtr>;

/// @brief Relation lists essentially act as substitution lists. They are 
///        introduced as a tool to make .quest file code more compact and 
///        readable.
class RelationList {
    /// @brief Relation list's unique name.
    const Str _name;

    /// @brief Relation list's unique ID.
    const ID _id;

    /// @brief Relation list's argument list. 
    ///        n-th argument (starting from 1) has (-n) ID.
    const ObjectVec _arguments;

    /// @brief Relation list's list of relations.
    /// All previously defined relation lists are replaced with their respective 
    /// relations.
    const StatementVec _statements;

public:
    RelationList(
            const Str& name,
            const ID id,
            const ObjectVec& arguments, 
            const StatementVec& statements
            ) noexcept;

    const StatementVec& getStatements() const noexcept;
    const ObjectVec& getArguments() const noexcept;
    const Str& getName() const noexcept;
    ID getId() const noexcept;

    /// @brief Evaluates the type compatibility with the given arguments.
    /// @param arguments Argument values.
    /// @return Returns `Result::OK()` if the given argument values are compatible.
    Result checkArgumentsCompatibility(const ObjectVec& arguments) const noexcept;

    /// @brief Builds a substituted array of relations. Very slow.
    /// @param arguments Relation list arguments for the substitution.
    /// @return Returns a newly constructed array of relations.
    StatementVec substitute(const ObjectVec& arguments) const noexcept;

    /// @brief A faster version of `substitute(...)` method.
    /// @param out An array of statements that is fully compatible with the 
    ///         `_statements` array. This implies that it contains the same 
    ///         relations in the same order, with identical constant arguments.
    /// @param arguments Relation list arguments for the substitution.
    void substituteFast(StatementVec& out, const ObjectVec& arguments) const noexcept;

};

}