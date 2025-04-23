// Copyright 2025 Pavlo Savchuk. Subject to the MIT license.

#pragma once

#include <libmozok/filesystem.hpp>

namespace mozok {
namespace app {

class StdFileSystem : public FileSystem {
public:
    StdFileSystem();
    Result getTextFile(const Str& path, Str& out) noexcept override;
};

}
}
