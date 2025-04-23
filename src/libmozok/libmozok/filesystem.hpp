// Copyright 2025 Pavlo Savchuk. Subject to the MIT license.

#pragma once

#include <libmozok/public_types.hpp>
#include <libmozok/result.hpp>

namespace mozok {

/// @brief File system interface through which `Server` can access text files
/// by their names.
class FileSystem {
public:
    FileSystem() 
    { /* empty */ }

    virtual ~FileSystem()
    { /* empty */ }

    /// @brief Gets the content of a text file by its path (name).
    /// @param path The path (or name) of the text file.
    /// @param out Outputs the content of the text file into this variable.
    /// @return The status of the operation.
    virtual mozok::Result getTextFile(
            const mozok::Str& path, 
            mozok::Str& /*out*/) noexcept {
        return mozok::Result::Error(
                "mozok::FileSystem::getTextFile(\"" 
                + path + "\"): Not implemented.");
    }
};

}
