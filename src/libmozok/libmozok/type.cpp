// Copyright 2024 Pavlo Savchuk. Subject to the MIT license.

#include <libmozok/type.hpp>

namespace mozok {

StrVec typesetToStrVec(const TypeSet& typeset) noexcept {
    StrVec res;
    for(const TypePtr& type : typeset)
        res.push_back(type->getName());
    return res;
}

bool areTypesetsCompatible(
        const TypeSet& bigger, 
        const TypeSet& smaller
        ) noexcept {
    if(smaller.size() > bigger.size())
        return false;
    for(const auto& fromSmaller : smaller)
        if(bigger.find(fromSmaller) == bigger.end())
            return false;
    return true;
}


Type::Type( const Str& name, 
            const ID id, 
            const TypeSet& supertypes
            ) noexcept :
    _name(name), 
    _id(id),
    _supertypes(supertypes)
    { /* empty */}

const TypeSet& Type::getSupertypes() const noexcept {
    return _supertypes;
}

const Str& Type::getName() const noexcept {
    return _name;
}

ID Type::getId() const noexcept {
    return _id;
}

}

