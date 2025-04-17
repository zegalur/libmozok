// ...

#pragma once

#include <libmozok/mozok.hpp>

namespace mozok {
namespace app {

struct DebugArg {
    enum Type {
        INT,
        STR,
        ANY
    };

    const Type type;
    const Str str;
    const int num;

    DebugArg() noexcept; // any
    DebugArg(const Str& str) noexcept;
    DebugArg(const int num) noexcept;
};

using DebugArgs = Vector<DebugArg>;

}
}
