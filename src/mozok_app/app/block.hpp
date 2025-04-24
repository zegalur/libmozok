// Copyright 2025 Pavlo Savchuk. Subject to the MIT license.

#pragma once

#include "app/command.hpp"
#include <libmozok/mozok.hpp>

namespace mozok {
namespace app {

class App;

/// @brief Debug block consisting of a type, name and a list of debug commands.
///        Split sub-blocks are separated by split debug commands.
class DebugBlock {

    /// @brief Block type.
    enum Type {
        EMPTY,  /// An empty block (usually a result of an parser error.)
        ACT,    /// Acts once per a timeline.
        ACT_IF, /// Acts only if a split block with the same name was acted.
        SPLIT,  /// Split block (sub-blocks are separated by split commands.)
        ALWAYS  /// Always acts.
    };

    const Type _type;
    const Str _name;
    const DebugCmdVec _cmds;

    /// @brief For split blocks, contains the split command indexes.
    const Vector<int> _splits;

    /// @return Builds the split command indexes array.
    static Vector<int> getSplits(const DebugCmdVec& cmds) noexcept;

    DebugBlock(
            const Type type,
            const Str& name,
            const DebugCmdVec& cmds
            ) noexcept;

    friend App;

public:
    static DebugBlock empty() noexcept;
    static DebugBlock act(const Str& name, const DebugCmdVec& cmds) noexcept;
    static DebugBlock act_if(const Str& name, const DebugCmdVec& cmds) noexcept;
    static DebugBlock split(const DebugCmdVec& cmds) noexcept;
    static DebugBlock always(const Str& name, const DebugCmdVec& cmds) noexcept;

    Str typeToStr() const noexcept;
};

}
}
