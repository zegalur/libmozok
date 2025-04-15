// Copyright 2024 Pavlo Savchuk. Subject to the MIT license.

#pragma once

#include <libmozok/mozok.hpp>


// TODO: 
//  [ ] Add: `FileReader` class as a part of mozok public interface with
//      `virtual Str FileReader::getTextFile(fileName)`
//  [ ] Add: `Result Server::load(script, FileReader)` that will do 
//      only the loading part of the script file, but not the debugging part.
//      (only commands `world` and `load`) - this will allow to use the same
//      .qsf file for both debugging the quests and defining the load order.

namespace mozok {
namespace app {

struct AppOptions {
    bool pauseOnError = false;
    Str printOnOk;
    Str serverName = "mozok_app";
    Str inputFileName;
    Str inputFile;
};


/// @brief ...
class App {
    AppOptions _options;
    Result _status;
    Server* _server;
    Str _stdWorldName;

private:
    App(const AppOptions& options) noexcept;
    
public:
    [[nodiscard]]
    static App* create(
        const AppOptions& options,
        Result& status
        ) noexcept;
    
    ~App() noexcept;


    const Result& getCurrentStatus() const;
    [[nodiscard]] Result newWorld(const Str& worldName) noexcept; 
    [[nodiscard]] Result setStdWorld(const Str& worldName) noexcept; 
    [[nodiscard]] Result unpause() noexcept;
};

}
}
