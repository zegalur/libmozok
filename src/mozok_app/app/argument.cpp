// Copyright 2025 Pavlo Savchuk. Subject to the MIT license.

#include "app/argument.hpp"
#include <string>

namespace mozok {
namespace app {

DebugArg::DebugArg() noexcept 
: type(Type::ANY), str(""), num(-1) 
{ /* empty */ }

DebugArg::DebugArg(const Str& _str) noexcept
: type(Type::STR), str(_str), num(-1)
{ /* empty */ }

DebugArg::DebugArg(const int _num) noexcept
: type(Type::INT), str(""), num(_num)
{ /* empty */ }

Str DebugArg::toStr() const noexcept {
    if(type == Type::ANY)
        return "_";
    if(type == Type::STR)
        return str;
    return std::to_string(num);
}

}
}
