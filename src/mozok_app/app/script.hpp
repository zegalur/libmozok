// Copyright 2025 Pavlo Savchuk. Subject to the MIT license.

// QSF (Quest Script File) parser.

#pragma once

#include <libmozok/mozok.hpp>

namespace mozok {
namespace app {

class App;

class QSFParser {
    QSFParser();
public:
    static Result parse(
            const Str& filename, 
            const Str& script, 
            App* app
            ) noexcept;

    static Result parseAndApplyCmd(
            const Str& command,
            App* app
            ) noexcept;
};

}
}
