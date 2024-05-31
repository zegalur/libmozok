// Copyright 2024 Pavlo Savchuk. Subject to the MIT license.

#pragma once

#include <string>
#include <vector>

namespace mozok {

/// @brief String type. 
/// Must be at least partially compatible with 'std::string'.
using Str = std::string;

/// @brief A vector of strings. 
/// Must be at least partially compatible with 'std::vector'.
using StrVec = std::vector<mozok::Str>;

/// @brief A vector type.
/// Must be at least partially compatible with 'std::vector'.
/// @tparam T The type of the vector elements.
template<typename T> using Vector = std::vector<T>;

}