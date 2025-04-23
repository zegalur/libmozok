// Copyright 2025 Pavlo Savchuk. Subject to the MIT license.

#include <libmozok/filesystem.hpp>
#include <libmozok/parser.hpp>

namespace mozok {

class Server;

/// @brief Parses the initialization part of QSF (Quest Script File) skipping the 
///     debug section and debug commands.
class QuestScriptParser_Base : public RecursiveDescentParser {
protected:
    Result _status;
    int _majorVersion;
    int _minorVersion;
    Str _scriptName;

    Result errorMsg(const Str& msg) const;
    Result errorMsg(const Str& msg, const int line) const;
    Result version() noexcept;
    Result scriptName() noexcept;
    Result worlds(Server* server) noexcept;
    Result world(Str& out) noexcept;
    Result filename(Str& filename) noexcept;
    Result project(Str& worldName, Str& projectFile) noexcept;
    Result projects(Server* server, FileSystem* fileSystem) noexcept;
    Result action(Str& worldName, Str& actionName, StrVec& arguments) noexcept;
    Result init(Server* server, bool applyInitActions) noexcept;

    /// @brief For parsing properly formatted `.qsf` file content.
    QuestScriptParser_Base(
            const Str& fileName,
            const Str& script);
    
    /// @brief For parsing one (debug) command.
    QuestScriptParser_Base(
            const Str& oneCommand);

    /// @brief Parses the header (initialization) part of a QSF File.
    /// @param server The server to which the commands will be applied.
    /// @param fileSystem Used to access text files by their names.
    /// @param applyInitActions If `true`, applies the actions from the 
    ///     `init:` section of the script.
    /// @return Returns the status of the operation.
    Result parseHeaderFunc(
            Server* server,
            FileSystem* fileSystem,
            bool applyInitActions
        ) noexcept;

public:
    /// @brief Parses the header (initialization) part of a QSF File into a server.
    /// @param server The server to which the commands will be applied.
    /// @param fileSystem Used to access text files by their names.
    /// @param fileName File name for error messages.
    /// @param script Script file text.
    /// @param applyInitActions If `true`, applies the actions from the 
    ///     `init:` section of the script.
    /// @return Returns the status of the operation.
    static Result parseHeader(
            Server* server,
            FileSystem* fileSystem,
            const Str& fileName,
            const Str& script,
            bool applyInitActions
        ) noexcept;

};

}
