// Copyright 2025 Pavlo Savchuk. Subject to the MIT license.

#pragma once

#include <libmozok/mozok.hpp>

namespace mozok {
namespace app {

class App;

/// @brief QSF parser that parses both initialization and debug commands.
class QSFParser {
    QSFParser();
public:
    /// @brief Parses QSF from the app options into the app.
    static Result parseAndInit(App* app) noexcept;

    /// @brief Parses and applies one debug command into the app.
    static Result parseAndApplyCmd(const Str& command, App* app) noexcept;
};

}
}
