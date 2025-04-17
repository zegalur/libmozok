// Copyright 2024 Pavlo Savchuk. Subject to the MIT license.

#include <libmozok/filesystem.hpp>
#include <libmozok/parser.hpp>

namespace mozok {

class Server;

/// @brief Parses the initial part of QSF (Quest Script File).
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
    Result project(Str& worldName, Str& projectFile) noexcept;
    Result projects(Server* server, FileSystem* fileSystem) noexcept;
    Result action(Str& worldName, Str& actionName, StrVec& arguments) noexcept;
    Result init(Server* server, bool applyInitActions) noexcept;

    QuestScriptParser_Base(
            const Str& fileName,
            const Str& script);
    
    QuestScriptParser_Base(
            const Str& oneCommand);

    Result parseHeaderFunc(
            Server* server,
            FileSystem* fileSystem,
            bool applyInitActions
        ) noexcept;

public:
    static Result parseHeader(
            Server* server,
            FileSystem* fileSystem,
            const Str& fileName,
            const Str& script,
            bool applyInitActions
        ) noexcept;

};

}
