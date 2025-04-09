// Copyright 2024 Pavlo Savchuk. Subject to the MIT license.

#pragma once

#include <libmozok/public_types.hpp>
#include <libmozok/private_types.hpp>
#include <libmozok/result.hpp>

#include <libmozok/relation.hpp>
#include <libmozok/object.hpp>

namespace mozok {

class Statement;
using StatementPtr = SharedPtr<Statement>;
using StatementVec = Vector<StatementPtr>;

struct StatementHash {
    std::size_t operator()(const StatementPtr& statement) const noexcept;
};
struct StatementEqual {
    bool operator()(const StatementPtr& a, const StatementPtr& b) const noexcept;
};

template<typename T>
using StatementMap = HashMap<StatementPtr, T, StatementHash, StatementEqual>;
using StatementSet = HashSet<StatementPtr, StatementHash, StatementEqual>;


/// @brief A statement is a relation applied to its arguments.
class Statement {
    /// @brief Statement's relation.
    const RelationPtr _relation;

    /// @brief Statement's arguments.
    ObjectVec _arguments;

    /// @brief A statement is considered constant if it contains only global 
    ///        objects and no variables.
    const bool _isConstant;

    /// @brief A statement is considered global if it refer to at least one
    ///        global object or it is a 0-arity relation.
    const bool _isGlobal;

    /// @brief Statement hash value.
    /// The statement hash function should be chosen so that the XOR combination 
    /// of such hash values remains relatively good as a hash value by itself.
    std::size_t _hash;

    bool isConstant(const ObjectVec& arguments) const noexcept;
    bool isGlobal(const ObjectVec& arguments) const noexcept;
    std::size_t computeHash() const noexcept;

public:
    Statement(const RelationPtr& relation, const ObjectVec& arguments) noexcept;
    
    /// @brief A statement is considered constant if it contains only global 
    ///        objects and no variables.
    /// @return Returns `true` if the statement is constant, and `false` otherwise.
    bool isConstant() const noexcept;

    /// @brief A statement is considered global if it refer to at least one
    ///        global object or it is a 0-arity relation.
    /// @return Returns `true` if the statement is global, and `false` otherwise.
    bool isGlobal() const noexcept;

    /// @brief Evaluates the type compatibility with the given arguments.
    /// @param arguments Argument values.
    /// @return Returns `Result::OK()` if the given argument values are compatible.
    Result checkArgumentsCompatibility(const ObjectVec& arguments) const noexcept;

    /// @brief Constructs a substituted statement. Variables with negative (-i) 
    ///        ID are replaced by the (-1-i)-th argument.
    /// WARNING! Arguments must be fully compatible with the statement's relation.
    /// @param arguments Arguments for the substitution.
    /// @return Returns a newly constructed statement.
    StatementPtr substitute(const ObjectVec& arguments) const noexcept;

    const RelationPtr& getRelation() const noexcept;
    const ObjectVec& getArguments() const noexcept;
    std::size_t getHash() const noexcept;

    /// @brief A non-constant version of the `getArguments`.
    /// WARNING! You should call the `recalculateHash()` function after 
    /// modifying the arguments.
    /// @return Statement's arguments.
    ObjectVec& getArguments() noexcept;

    /// @brief Recalculates the hash value.
    void recalculateHash() noexcept;
};


}