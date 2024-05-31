// Copyright 2024 Pavlo Savchuk. Subject to the MIT license.

#pragma once

#include <libmozok/public_types.hpp>
#include <libmozok/private_types.hpp>
#include <libmozok/result.hpp>

#include <libmozok/type.hpp>
#include <libmozok/object.hpp>

namespace mozok {

class Relation;
using RelationPtr = SharedPtr<const Relation>;
using RelationVec = Vector<RelationPtr>;

/// @brief A quest relation is a tool for representing relationships between 
///        objects.
/// We use relations together with objects to state facts about the quest world 
/// or conditions. However, unlike predicates in first-order logic, our 
/// relations are typed. For example, a relation `Alive(Human)` indicates that 
/// objects classified as `Human` can be `Alive`. If we have an object 
/// `player : Human`, we can construct a statement `Alive(player)`.
class Relation {
    /// @brief Relation's unique name.
    const Str _name;

    /// @brief Relation's unique ID.
    const ID _id;

    /// @brief Relation's argument types.
    const TypeVec _argTypes;

public:
    Relation(const Str& name, 
             const ID id, 
             const TypeVec& argTypes
             ) noexcept;

    ID getId() const noexcept;
    const Str& getName() const noexcept; 

    /// @brief Evaluates the type compatibility with the given arguments.
    /// @param arguments Argument values.
    /// @return Returns `Result::OK()` if the given argument values are compatible.
    Result checkArgumentsCompatibility(const ObjectVec& arguments) const noexcept;
};

}