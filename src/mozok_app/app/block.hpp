// ...

#pragma once

#include "app/command.hpp"

#include <libmozok/mozok.hpp>

namespace mozok {
namespace app {

class DebugBlock {
    enum Type {
        EMPTY, // ...
        ACT, // ...
        SPLIT, // ...
        ALWAYS // ...
    };

    const Type _type;
    const Str _name;
    const DebugCmdVec _cmds;

    DebugBlock(
            const Type type,
            const Str& name,
            const DebugCmdVec& cmds
            ) noexcept;

    friend App;

public:
    static DebugBlock empty() noexcept;
    static DebugBlock act(const Str& name, const DebugCmdVec& cmds) noexcept;
    static DebugBlock split(const DebugCmdVec& cmds) noexcept;
    static DebugBlock always(const Str& name, const DebugCmdVec& cmds) noexcept;
};

}
}
