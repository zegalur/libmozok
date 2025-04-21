// Copyright 2025 Pavlo Savchuk. Subject to the MIT license.

#include "app/argument.hpp"

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

}
}
