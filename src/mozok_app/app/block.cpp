// ...

#include "app/block.hpp"

namespace mozok {
namespace app {

DebugBlock::DebugBlock(
        const Type type,
        const Str& name, 
        const DebugCmdVec& cmds
        ) noexcept :
    _type(type),
    _name(name),
    _cmds(cmds)
{ /* empty */ }

DebugBlock DebugBlock::empty() noexcept {
    return DebugBlock(Type::EMPTY, "???", {});
}

DebugBlock DebugBlock::act(const Str& name, const DebugCmdVec& cmds) noexcept {
    return DebugBlock(Type::ACT, name, cmds);
}

DebugBlock DebugBlock::split(const DebugCmdVec& cmds) noexcept {
    return DebugBlock(Type::SPLIT, "???", cmds);
}

DebugBlock DebugBlock::always(const Str& name, const DebugCmdVec& cmds) noexcept {
    return DebugBlock(Type::ALWAYS, name, cmds);
}

}
}
