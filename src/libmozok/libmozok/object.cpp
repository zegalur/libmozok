// Copyright 2024 Pavlo Savchuk. Subject to the MIT license.

#include <libmozok/object.hpp>

namespace mozok {

Object::Object(
        const Str& name, 
        const ID id, 
        const TypeSet& types
        ) noexcept :
    _name(name),
    _id(id),
    _types(types)
{ /* empty */ }

const Str& Object::getName() const noexcept {
    return _name;
}

const ID& Object::getId() const noexcept {
    return _id;
}

const TypeSet& Object::getTypeSet() const noexcept {
    return _types;
}

}