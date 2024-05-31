// Copyright 2024 Pavlo Savchuk. Subject to the MIT license.

#pragma once

#include <libmozok/public_types.hpp>
#include <libmozok/private_types.hpp>
#include <libmozok/result.hpp>

namespace mozok {

class Type;
using TypePtr = SharedPtr<const Type>;
using TypeSet = UnorderedSet<TypePtr>;
using TypeVec = Vector<TypePtr>;


/// @brief Constructs a vector of strings containing the names of the types from 
///        the type set.
/// @param typeset A type set.
/// @return Returns a vector of strings containing the names of the types from 
///         the type set.
StrVec typesetToStrVec(const TypeSet& typeset) noexcept;

/// @brief Checks if two type sets are compatible 
///        (if the larger set contains the smaller set).
/// @param bigger The bigger type set.
/// @param smaller The smaller type set.
/// @return Returns `true` if the bigger type set contains the smaller. 
///         Returns `false` otherwise.
bool areTypesetsCompatible(
        const TypeSet& bigger, 
        const TypeSet& smaller
        ) noexcept;


/// @brief This class represents a type of quest object. Supports subtyping.
/// An object can belong to multiple types. Types serve three main purposes:
///   1. They enable a type checker that aids in identifying certain bugs. 
///   2. Types are utilized for optimization purposes. 
///   3. Types facilitate logical organization of the quest world.
class Type {
    /// @brief Type's unique name.
    const Str _name;

    /// @brief Type's unique ID.
    const ID _id;

    /// @brief Type's supertypes typeset.
    const TypeSet _supertypes;

public:
    Type( const Str& name, 
          const ID id, 
          const TypeSet &supertypes
          ) noexcept;

    const TypeSet& getSupertypes() const noexcept;
    const Str& getName() const noexcept;
    ID getId() const noexcept;
    
};

}