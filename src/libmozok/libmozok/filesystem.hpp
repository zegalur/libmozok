// Copyright 2024 Pavlo Savchuk. Subject to the MIT license.

#pragma once

#include <libmozok/public_types.hpp>
#include <libmozok/result.hpp>

namespace mozok {

class FileSystem {
public:
    FileSystem() 
    { /* empty */ }

    virtual ~FileSystem()
    { /* empty */ }

    virtual mozok::Result getTextFile(
            const mozok::Str& path, 
            mozok::Str& /*out*/) {
        return mozok::Result::Error(
                "mozok::FileSystem::getTextFile(\"" 
                + path + "\"): Not implemented.");
    }
};

}
