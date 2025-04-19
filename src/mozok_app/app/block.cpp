// ...

#include "app/block.hpp"
#include "app/command.hpp"

namespace mozok {
namespace app {

DebugBlock::DebugBlock(
        const Type type,
        const Str& name, 
        const DebugCmdVec& cmds
        ) noexcept :
    _type(type),
    _name(name),
    _cmds(cmds),
    _splits(getSplits(cmds))
{ /* empty */ }

Vector<int> DebugBlock::getSplits(const DebugCmdVec& cmds) noexcept {
    Vector<int> res;
    for(DebugCmdVec::size_type i=0; i<cmds.size(); ++i)
        if(cmds[i]._cmd == DebugCmd::SPLIT)
            res.push_back(i);
    return res;
}

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
