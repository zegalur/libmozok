#include "libmozok/message_processor.hpp"
#include <libmozok/script.hpp>
#include <libmozok/error_utils.hpp>
#include <libmozok/server.hpp>

#include <strstream>

namespace mozok {

namespace {

const int MAJOR_VERSION = 1;
const int MINOR_VERSION = 0;

const char* KEYWORD_VERSION = "version";
const char* KEYWORD_SCRIPT = "script";
const char* KEYWORD_WORLDS = "worlds";
const char* KEYWORD_PROJECTS = "projects";
const char* KEYWORD_INIT = "init";

}

Result QuestScriptParser_Base::errorMsg(const Str& msg) const {
    std::strstream ss;
    ss << _file << ":" << (_line + 1) << ":" << (_col + 1) << ": " << msg;
    return Result::Error(ss.str());
}

Result QuestScriptParser_Base::errorMsg(const Str& msg, const int line) const {
    std::strstream ss;
    ss << _file << ":" << (line + 1) << ":1: " << msg;
    return Result::Error(ss.str());
}

Result QuestScriptParser_Base::version() noexcept {
    Result res;
    res <<= keyword(KEYWORD_VERSION);
    res <<= space(1);
    res <<= pos_int(_majorVersion);
    res <<= space(1);
    res <<= pos_int(_minorVersion);
    res <<= empty_lines();
    if(_majorVersion != MAJOR_VERSION || _minorVersion != MINOR_VERSION)
        return errorParserUnsupportedVersion(
                _file, _line, _col,
                MAJOR_VERSION, MINOR_VERSION,
                _majorVersion, _minorVersion);
    return res;
}

Result QuestScriptParser_Base::scriptName() noexcept {
    Result res;
    res <<= keyword(KEYWORD_SCRIPT);
    res <<= space(1);
    res <<= name(_scriptName);
    res <<= empty_lines();
    return res;
}

Result QuestScriptParser_Base::worlds(Server* server) noexcept {
    Result res;
    res <<= keyword(KEYWORD_WORLDS);
    res <<= colon_with_spaces();
    res <<= next_line();
    res <<= empty_lines();
    StrVec worlds;
    int curLine = _line + 1;
    res <<= name_list(worlds, BOTH);
    res <<= empty_lines();
    if(res.isError())
        return res;
    for(StrVec::size_type i=0; i < worlds.size(); ++i, ++curLine) {
        res <<= server->createWorld(worlds[i]);
        if(res.isError()) {
            res <<= errorMsg(
                    "Can't create `" + worlds[i] + "` world", curLine);
            break;
        }
    }
    return res;
}

Result QuestScriptParser_Base::world(Str& out) noexcept {
    Result res;
    res <<= bracket_open();
    if(res.isError())
        return res;
    res <<= name(out);
    if(res.isError())
        return res;
    res <<= bracket_close();
    return res;
}

Result QuestScriptParser_Base::filename(Str& filename) noexcept {
    int lineEnd = _pos;
    for(; _src[lineEnd] != '\0' && _src[lineEnd] != '\n'; ++lineEnd);
    // remove empty space on the right side of the string
    for(; lineEnd > _pos; --lineEnd) {
        bool empty = false;
        empty |= _src[lineEnd] == ' ';
        empty |= _src[lineEnd] == '\t';
        empty |= _src[lineEnd] == '\r';
        empty |= _src[lineEnd] == '\n';
        empty |= _src[lineEnd] == '\0';
        if(empty == false)
            break;
    }
    filename = _text.substr(_pos, lineEnd - _pos + 1);
    if(filename.length() == 0)
        return errorMsg("Expecting a non-empty file name");
    _col += (lineEnd - _pos + 1);
    _pos += (lineEnd - _pos + 1);
    return Result::OK();
}

Result QuestScriptParser_Base::project(Str& worldName, Str& projectFile) noexcept {
    Result res;
    res <<= world(worldName);
    res <<= space(1);
    if(res.isError())
        return res;
    // the rest is the project file name
    res <<= filename(projectFile);
    return res;
}

Result QuestScriptParser_Base::projects(
        Server* server, 
        FileSystem* fileSystem
        ) noexcept {
    Result res;
    res <<= keyword(KEYWORD_PROJECTS);
    res <<= colon_with_spaces();
    res <<= next_line();
    res <<= empty_lines();
    while(space(1).isOk()) {
        Str worldName, projectFile;
        res <<= project(worldName, projectFile);
        if(res.isError())
            return res;
        Str projectFileText;
        res <<= fileSystem->getTextFile(projectFile, projectFileText);
        if(res.isError())
            return res;
        res <<= server->addProject(worldName, projectFile, projectFileText);
        if(res.isError()) {
            res <<= errorMsg(
                    "Error while loading a project `" + projectFile 
                    + "` into [" + worldName + "] world.");
            return res;
        }
        res <<= empty_lines();
    }
    return res;
}

Result QuestScriptParser_Base::action(
        Str& worldName, 
        Str& actionName, 
        StrVec& arguments) noexcept {
    Result res;
    arguments.clear();
    res <<= world(worldName);
    res <<= space(1);
    if(res.isError())
        return res;
    res <<= name(actionName, UPPER);
    res <<= space(0);
    if(res.isError())
        return res;
    res <<= par_open();
    res <<= space(0);
    if(res.isError())
        return res;
    if(_src[_pos] == ')')
        // Zero-arity action
        return res <<= par_close();
    Str tmp;
    res <<= name(tmp, LOWER);
    res <<= space(0);
    if(res.isError())
        return res;
    arguments.push_back(tmp);
    while(comma().isOk()) {
        res <<= space(0);
        res <<= name(tmp, LOWER);
        if(res.isError())
            return res;
        arguments.push_back(tmp);
        res <<= space(0);
    }
    res <<= par_close();
    return res;
}

Result QuestScriptParser_Base::init(
        Server* server, 
        bool applyInitActions
        ) noexcept {
    Result res;
    ActionError actionError;
    res <<= keyword(KEYWORD_INIT);
    res <<= colon_with_spaces();
    res <<= next_line();
    res <<= empty_lines();
    while(space(1).isOk()) {
        Str worldName;
        Str actionName;
        StrVec args;
        res <<= action(worldName, actionName, args);
        if(res.isError())
            return res;
        if(applyInitActions)
            res <<= server->applyAction(worldName, actionName, args, actionError);
        if(res.isError())
            return res <<= errorMsg("Incorrect init action.");
        res <<= empty_lines();
    }
    return res;
}

Result QuestScriptParser_Base::parseHeaderFunc(
        Server* server,
        FileSystem* fileSystem,
        bool applyInitActions
        ) noexcept {
    _status <<= empty_lines();
    if(_status.isOk())
        _status <<= version();
    if(_status.isOk())
        _status <<= scriptName();
    if(_status.isOk())
        _status <<= worlds(server);
    if(_status.isOk())
        _status <<= projects(server, fileSystem);
    if(_status.isOk())
        _status <<= init(server, applyInitActions);
    return _status;
}

QuestScriptParser_Base::QuestScriptParser_Base(
        const Str& fileName,
        const Str& script) :
RecursiveDescentParser(fileName, script, true),
_majorVersion(0),
_minorVersion(0),
_scriptName("???")
{ /* empty */ }

QuestScriptParser_Base::QuestScriptParser_Base(
        const Str& oneCommand) :
RecursiveDescentParser("[debug_terminal]", oneCommand, false),
_majorVersion(MAJOR_VERSION),
_minorVersion(MINOR_VERSION),
_scriptName("[debug_terminal]")
{ /* empty */ }


Result QuestScriptParser_Base::parseHeader(
        Server *server, 
        FileSystem* fileSystem,
        const Str &fileName, 
        const Str &script, 
        bool applyInitActions
        ) noexcept {
    QuestScriptParser_Base parser(fileName, script);
    Result res = parser.parseHeaderFunc(server, fileSystem, applyInitActions);
    return res;
}

}
