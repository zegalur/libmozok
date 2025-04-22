// Copyright 2025 Pavlo Savchuk. Subject to the MIT license.

#pragma once

#include <libmozok/public_types.hpp>
#include <libmozok/mozok.hpp>

namespace mozok {
namespace app {

/// @brief Debug command argument.
struct DebugArg {

    /// @brief Debug command argument type.
    enum Type {
        INT, /// Integer value (stored in `num`).
        STR, /// String value (stored in `str`).
        ANY  /// Any "_" value
    };

    const Type type;
    const Str str;
    const int num;

    DebugArg() noexcept; // any
    DebugArg(const Str& str) noexcept;
    DebugArg(const int num) noexcept;

    Str toStr() const noexcept;
};

using DebugArgs = Vector<DebugArg>;


// ========================================================================= //

// Templates to automate the pattern matching (see `match()` function).

template<int L, typename... Ts>
struct GetLen;

template<int L>
struct GetLen<L, StrVec> {
    static inline int get_len(const StrVec& strs) noexcept {
        return L + strs.size();
    }
};
template<int L>
struct GetLen<L> {
    static inline int get_len() noexcept {
        return L;
    }
};
template<int L, typename T, typename... Ts>
struct GetLen<L, T, Ts...> {
    static inline int get_len(const T&, const Ts&... args) noexcept {
        return GetLen<L+1, Ts...>::get_len(args...);
    }
};

template<int Indx, typename... Ts>
struct MatchClass;

template<int Indx, typename... Ts>
struct MatchClass<Indx, StrVec, Ts...> {
    static inline bool match_fn(
            const DebugArgs& args, 
            const StrVec& strs, 
            const Ts&... args2
            ) noexcept {
        if(strs.size() == 0)
            return MatchClass<Indx,Ts...>::match_fn(args, args2...);
        StrVec strsCopy(strs.begin()+1, strs.end());
        if(args[Indx].type == DebugArg::ANY)
            return MatchClass<Indx+1,Ts...>::match_fn(args, args2...);
        if(args[Indx].type != DebugArg::STR)
            return false;
        return args[Indx].str == strs.front() 
            && MatchClass<Indx+1,Ts...>::match_fn(args, args2...);
    }
};
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

/// @brief Checks if debug command arguments matches the input arguments. 
template<typename... Ts>
inline bool match(const DebugArgs& args1, const Ts&... args2) noexcept {
    const DebugArgs::size_type l = DebugArgs::size_type(
            GetLen<0, Ts...>::get_len(args2...));
    if(args1.size() != l)
        return false;
    if(l == 0)
        return true;
    return MatchClass<0, Ts...>::match_fn(args1, args2...);
}

}
}
