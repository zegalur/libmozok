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


// Templates to make the pattern matching

template<int Indx, typename... Ts>
struct MatchClass;

template<int Indx, typename... Ts>
struct MatchClass<Indx, Str, Ts...> {
    static inline bool match_fn(
            const DebugArgs& args, 
            const Str& str, 
            const Ts&... args2
            ) noexcept {
        if(args[Indx].type == DebugArg::ANY)
            return MatchClass<Indx+1,Ts...>::match_fn(args, args2...);
        if(args[Indx].type != DebugArg::STR)
            return false;
        return args[Indx].str == str 
            && MatchClass<Indx+1,Ts...>::match_fn(args, args2...);
    }
};

template<int Indx, typename... Ts>
struct MatchClass<Indx, int, Ts...> {
    static inline bool match_fn(
            const DebugArgs& args, 
            const int& num, 
            const Ts&... args2
            ) noexcept {
        if(args[Indx].type == DebugArg::ANY)
            return MatchClass<Indx+1,Ts...>::match_fn(args, args2...);
        if(args[Indx].type != DebugArg::INT)
            return false;
        return args[Indx].num == num 
            && MatchClass<Indx+1,Ts...>::match_fn(args, args2...);
    }
};

template<int Indx>
struct MatchClass<Indx> {
    static inline bool match_fn(
            const DebugArgs& /*args*/
            ) noexcept {
        return true;
    }
};

template<typename... Ts>
inline bool match(const DebugArgs& args1, const Ts&... args2) noexcept {
    constexpr DebugArgs::size_type l = DebugArgs::size_type(sizeof...(Ts));
    if(args1.size() != l)
        return false;
    if(l == 0)
        return true;
    return MatchClass<0, Ts...>::match_fn(args1, args2...);
}

}
}
