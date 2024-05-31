// Copyright 2024 Pavlo Savchuk. Subject to the MIT license.

#include <libmozok/relation.hpp>

#include <libmozok/error_utils.hpp>

namespace mozok {

Relation::Relation(
        const Str& name, 
        const ID id, 
        const TypeVec& argTypes
        ) noexcept :
    _name(name),
    _id(id),
    _argTypes(argTypes) 
{ /* empty */ }

ID Relation::getId() const noexcept {
    return _id;
}

const Str& Relation::getName() const noexcept {
    return _name;
}

Result Relation::checkArgumentsCompatibility(const ObjectVec& arguments) const noexcept {
    if(arguments.size() != _argTypes.size())
        return errorRelArgError_InvalidArity(
                _name, (int)arguments.size(), (int)_argTypes.size());

    for(ObjectVec::size_type i = 0; i < arguments.size(); ++i) {
        const ObjectPtr arg = arguments[i];
        const TypeSet argTypeSet = arg->getTypeSet();
        if(argTypeSet.find(_argTypes[i]) == argTypeSet.end())
            return errorRelArgError_InvalidType(
                    _name, i, arg->getName(), 
                    typesetToStrVec(argTypeSet), 
                    _argTypes[i]->getName());
    }

    return Result::OK();
}

}