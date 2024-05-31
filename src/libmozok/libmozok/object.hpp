// Copyright 2024 Pavlo Savchuk. Subject to the MIT license.

#pragma once

#include <libmozok/public_types.hpp>
#include <libmozok/private_types.hpp>
#include <libmozok/result.hpp>

#include <libmozok/type.hpp>

namespace mozok {

class Object;
using ObjectPtr = SharedPtr<const Object>;
using ObjectVec = Vector<ObjectPtr>;
using ObjectSet = UnorderedSet<ObjectPtr>;

/// @brief Quest world objects.
/// Objects, along with the relationships between them form a state. Typically, 
/// these correspond to in-game entities such as players, keys, bosses, etc.
/// They are the "nouns" of the quest world.
class Object {
    /// @brief The unique name of the object (typically starting with a 
    ///        lowercase letter.
    const Str _name;

    /// @brief The unique ID of the object (negative for variables).
    const ID _id;

    /// @brief The complete type of the object, encompassing all possible types 
    ///        it could belong to
    const TypeSet _types;

public:
    Object(const Str& name, const ID id, const TypeSet& types) noexcept;
    
    const Str& getName() const noexcept;
    const ID& getId() const noexcept;
    const TypeSet& getTypeSet() const noexcept;
};

}