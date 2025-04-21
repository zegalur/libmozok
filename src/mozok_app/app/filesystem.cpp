// Copyright 2025 Pavlo Savchuk. Subject to the MIT license.

#include "app/filesystem.hpp"

#include <fstream>
#include <iterator>

namespace mozok {
namespace app {

StdFileSystem::StdFileSystem() : FileSystem() 
{ /* empty */ }

Result StdFileSystem::getTextFile(const Str& path, Str& out) noexcept {
    std::ifstream file(path);
    if (file.is_open()) {
        std::string src = std::string(
                std::istreambuf_iterator<char>(file),
                std::istreambuf_iterator<char>());
        file.close();
        out = src;
        return Result::OK();
    }
    return Result::Error("StdFileSystem: Can't open `" + path +"` file.");
}

}
}
