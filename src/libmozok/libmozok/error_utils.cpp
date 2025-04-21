// Copyright 2024 Pavlo Savchuk. Subject to the MIT license.

#include <libmozok/error_utils.hpp>

#include <string>
#include <sstream>

namespace mozok {

Result makeFileLineError(const char* file, int line, const Str& text) {
    std::stringstream strStream;
    strStream << text << " [" << file << "(" << line << ")]";
    return Result::Error(strStream.str());
}

Result makeFileLineColError(const char* file, int line, int col, const Str& msg) {
    std::stringstream strStream;
    strStream << msg << " [" << file << ":" << line << ":" << col << "]";
    return Result::Error(strStream.str());
}

Result errorNotImplemented(const char* file, int line, const char* func) noexcept {
    return makeFileLineError(
            file, line, "`" + Str(func) + "` is not implemented.");
}

Result arityError(
        const Str& kind, // "Action", "Relation"
        const Str& name, 
        const int expectedArity, 
        const int givenArity
        ) noexcept {
    std::stringstream strStream;
    strStream << kind << " '" << name << "' ";
    strStream << "expecting " << expectedArity << " arguments but ";
    strStream << givenArity << " arguments were given.";
    return Result::Error(strStream.str());
}

Str strVecToStr(const StrVec& vec) noexcept {
    std::stringstream res;
    for (StrVec::size_type i = 0; i < vec.size(); ++i)
        res << vec[i] << (i != vec.size()-1 ? "," : "");
    return res.str();
}

Result invalidArgumentType(
        const Str& kind, // "Action", "Relation"
        const Str& name,
        const int indx, 
        const Str& argObjName,
        const StrVec& argObjType,
        const StrVec& expectedType
        ) noexcept {
    std::stringstream strStream;
    strStream << kind << " '" << name << "' ";
    strStream << (indx+1) << "-th argument '" << argObjName << "' ";
    strStream << "has an incompatible type ('" << strVecToStr(argObjType);
    strStream << "'). Expected an object compatible ";
    strStream << "with '" << strVecToStr(expectedType) << "' type.";
    return Result::Error(strStream.str());
}


// ================================= Parser ================================= //

Result errorParserError(
        const Str& fileName, int line, int col, const Str& msg) noexcept {
    return makeFileLineColError(fileName.c_str(), line+1, col+1, msg);
}

Result errorParserWorldError(
        const Str& fileName, int line, int col, const Str& serverWorldName) noexcept {
    return errorParserError(
            fileName, line, col, "([Server]:[World])(" + serverWorldName + 
            ") error. See the previous error message for the details.");
}

Result errorParserUnsupportedVersion(const Str& fileName, int line, int col, 
        int curMajor, int curMinor, int badMajor, int badMinor) noexcept {
    std::stringstream strStream;
    strStream << "Version (" << badMajor << ".";
    strStream << badMinor << ") is not supported. ";
    strStream << "Expected version (" << curMajor << "." << curMinor << ").";
    return errorParserError(fileName, line, col, strStream.str());
}

Result errorExpectingKeyword(
        const Str& fileName, int line, int col, const Str& keyword) noexcept {
    return errorParserError(
            fileName, line, col, "Expecting `" + keyword + "` keyword.");
}

Result errorExpectingSpace(const Str& fileName, int line, int col) noexcept {
    return errorParserError(fileName, line, col, "Expecting space symbol(s).");
}

Result errorExpectingNewLine(const Str& fileName, int line, int col) noexcept {
    return errorParserError(fileName, line, col, "Expecting new line.");
}

Result errorExpectingDigit(const Str& fileName, int line, int col) noexcept {
    return errorParserError(fileName, line, col, "Expecting a digit symbol.");
}

Result errorExpectingUppercase(const Str& fileName, int line, int col) noexcept {
    return errorParserError(fileName, line, col, "Expecting an uppercase letter.");
}

Result errorExpectingLowercase(const Str& fileName, int line, int col) noexcept {
    return errorParserError(fileName, line, col, "Expecting an lowercase letter.");
}

Result errorExpectingUnderscore(const Str& fileName, int line, int col) noexcept {
    return errorParserError(fileName, line, col, "Expecting an underscore symbol.");
}

Result errorExpectingLetter(const Str& fileName, int line, int col) noexcept {
    return errorParserError(fileName, line, col, "Expecting a letter.");
}

Result errorExpectingColon(const Str& fileName, int line, int col) noexcept {
    return errorParserError(fileName, line, col, "Expecting a colon `:` symbol.");
}

Result errorExpectingComma(const Str& fileName, int line, int col) noexcept {
    return errorParserError(fileName, line, col, "Expecting a comma `,` symbol.");
}

Result errorInvalidKeyword(
        const Str& fileName, int line, int col, const Str& keyword) noexcept {
    return errorParserError(fileName, line, col, 
            "Invalid/Unsupported keyword `" + keyword + "`.");
}

Result errorExpectingOpenPar(const Str& fileName, int line, int col) noexcept {
    return errorParserError(fileName, line, col, "Expecting open parenthesis `(` symbol.");
}

Result errorExpectingClosePar(const Str& fileName, int line, int col) noexcept {
    return errorParserError(fileName, line, col, "Expecting close parenthesis `)` symbol.");
}

Result errorExpectingOpenBracket(const Str& fileName, int line, int col) noexcept {
    return errorParserError(fileName, line, col, "Expecting open braket `[` symbol.");
}

Result errorExpectingCloseBracket(const Str& fileName, int line, int col) noexcept {
    return errorParserError(fileName, line, col, "Expecting close bracket `]` symbol.");
}


// ================================ Server ================================== //

Result errorServerWorkerIsRunning(const Str& serverName) noexcept {
    return Result::Error("[" + serverName +"] : Not allowed while the worker thread is running.");
}


// ================================= World ================================== //

Result errorWorldAlreadyExists(const Str& serverName, const Str& worldName) noexcept {
    return Result::Error(
        "World named `" + worldName + 
        "` already exists. Server=`" + serverName + "`");
}

Result errorWorldDoesntExist(const Str& serverName, const Str& worldName) noexcept {
    return Result::Error(
        "World named `" + worldName + 
        "` doesn't exist. Server=`" + serverName + "`");
}

Result errorWorldOtherError(const Str& serverWorldName, const Str& msg) noexcept {
    return Result::Error("([Server]:[World])(" + serverWorldName + ") " + msg);
}

Result errorCantApplyNAAction(
        const Str& serverWorldName, const Str& actionName) noexcept {
    return errorWorldOtherError(serverWorldName, 
            "Can't apply N/A action '" + actionName + "'");
}


// ================================= Type =================================== //

Result errorTypeError(const Str& serverWorldName, const Str& msg) noexcept {
    return Result::Error("([Server]:[World])(" + serverWorldName + ") " + msg);
}

Result errorTypeAlreadyExists(const Str& serverWorldName, const Str& typeName) noexcept {
    return errorTypeError(serverWorldName, "Type `" + typeName + "` already exists.");
}

Result errorUndefinedType(const Str& serverWorldName, const Str& typeName) noexcept {
    return errorTypeError(serverWorldName, "Undefined type `" + typeName + "`.");
}

Result errorTypeCantDefine(const Str& serverWorldName, const Str& typeName) noexcept {
    return errorTypeError(serverWorldName, 
            "Can't define type `" + typeName + "`. See the previous error.");
}


// ================================ Object ================================== //

Result errorObjectError(const Str& serverWorldName, const Str& msg) noexcept {
    return Result::Error("([Server]:[World])(" + serverWorldName + ") " + msg);
}

Result errorObjectAlreadyExists(const Str& serverWorldName, const Str& objectName) noexcept {
    return errorObjectError(serverWorldName, "Object `" + objectName + "` already exists.");
}

Result errorUndefinedObject(const Str& serverWorldName, const Str& objectName) noexcept {
    return errorObjectError(serverWorldName, "Undefined object `" + objectName + "`.");
}

Result errorObjectCantDefine(const Str& serverWorldName, const Str& objectName) noexcept {
    return errorObjectError(serverWorldName, 
            "Can't define object `" + objectName + "`. See the previous error.");
}


// =============================== Relation ================================= //

Result errorRelationError(const Str& serverWorldName, const Str& msg) noexcept {
    return Result::Error("([Server]:[World])(" + serverWorldName + ") " + msg);
}

Result errorRelAlreadyExists(const Str& serverWorldName, const Str& relationName) noexcept {
    return errorRelationError(serverWorldName, "Relation `" + relationName + "` already exists.");
}

Result errorUndefinedRel(const Str& serverWorldName, const Str& relationName) noexcept {
    return errorRelationError(serverWorldName, "Undefined relation `" + relationName + "`.");
}

Result errorRelArgError_InvalidArity(
        const Str& relationName, 
        const int expectedArity, 
        const int givenArity
        ) noexcept {
    return arityError("Relation", relationName, expectedArity, givenArity);
}

Result errorRelArgError_InvalidType(
        const Str& relationName, 
        const int indx, 
        const Str& argObjName,
        const StrVec& argObjType,
        const Str& expectedType
        ) noexcept {
    return invalidArgumentType(
        "Relation", relationName, indx, argObjName, argObjType, {expectedType});
}

Result errorRelationCantDefine(const Str& serverWorldName, const Str& relationName) noexcept {
    return errorRelationError(serverWorldName, 
            "Can't define relation `" + relationName + "`. See the previous error.");
}


// ============================= Relation List ============================== //

Result errorRListError(const Str& serverWorldName, const Str& msg) noexcept {
    return Result::Error("([Server]:[World])(" + serverWorldName + ") " + msg);
}

Result errorRListAlreadyExists(
        const Str& serverWorldName, const Str& relationListName) noexcept {
    return errorRListError(
            serverWorldName, "Relation list `" + relationListName + "` already exists.");
}

Result errorUndefinedRList(
        const Str& serverWorldName, const Str& relationListName) noexcept {
    return errorRListError(
            serverWorldName, "Undefined relation list `" + relationListName + "`.");
}

Result errorRListArgError_InvalidArity(
        const Str& rlistName, 
        const int expectedArity, 
        const int givenArity
        ) noexcept {
    return arityError("Relation List", rlistName, expectedArity, givenArity);
}

Result errorRListArgError_InvalidType(
        const Str& rlistName, 
        const int indx, 
        const Str& argObjName,
        const StrVec& argObjType,
        const StrVec& expectedType
        ) noexcept {
    return invalidArgumentType(
        "Relation List", rlistName, indx, argObjName, argObjType, expectedType);
}

Result errorRListCantDefine(const Str& serverWorldName, const Str& rlistName) noexcept {
    return errorRListError(serverWorldName, 
            "Can't define relation list `" + rlistName + "`. See the previous error.");
}


// ================================ Action ================================== //

Result errorActionError(const Str& serverWorldName, const Str& msg) noexcept {
    return Result::Error("([Server]:[World])(" + serverWorldName + ") " + msg);
}

Result errorActionAlreadyExists(const Str& serverWorldName, const Str& actionName) noexcept {
    return errorActionError(
            serverWorldName, "Action `" + actionName + "` already exists.");
}

Result errorUndefinedAction(const Str& serverWorldName, const Str& actionName) noexcept {
    return errorActionError(
        serverWorldName, "Action `" + actionName + "` is undefined.");
}

Result errorActionCantDefine(const Str& serverWorldName, const Str& actionName) noexcept {
    return errorActionError(serverWorldName, 
            "Can't define action `" + actionName + "`. See the previous error.");
}

Result errorActionPreError() noexcept {
    return Result::Error("Action `pre` section error. See the previous error.");
}

Result errorActionRemError() noexcept {
    return Result::Error("Action `rem` section error. See the previous error.");
}

Result errorActionAddError() noexcept {
    return Result::Error("Action `add` section error. See the previous error.");
}

Result errorActionArgError_InvalidArity(
        const Str& actionName, 
        const int expectedArity, 
        const int givenArity
        ) noexcept {
    return arityError("Action", actionName, expectedArity, givenArity);
}

Result errorActionArgError_InvalidType(
        const Str& actionName, 
        const int indx, 
        const Str& argObjName, 
        const StrVec& argObjType,
        const StrVec& expectedType
        ) noexcept {
    return invalidArgumentType(
        "Action", actionName, indx, argObjName, argObjType, expectedType);
}

Result errorActionPreconditionsFailed(
        const Str& serverWorldName, const Str& actionName) noexcept {
    return errorActionError(
        serverWorldName, "Action `" + actionName 
                                    + "` preconditions have not been met.");
}

Result errorActionSetStatusGoalError(
        const Str& serverWorldName, 
        const Str& actionName, 
        const Str& questName, 
        const int errorGoalIndx
        ) noexcept {
    return errorActionError(serverWorldName, 
            "Action `" + actionName + "` set status command goal index `"
            + Str(std::to_string(errorGoalIndx)) + "` of a quest `"
            + questName + "` is invalid");
}

Result errorActionSetStatusParentGoalError(
        const Str& serverWorldName, 
        const Str& actionName, 
        const Str& questName, 
        const int errorGoalIndx
        ) noexcept {
    return errorActionError(serverWorldName, 
            "Action `" + actionName + "` set status command goal index `"
            + Str(std::to_string(errorGoalIndx)) + "` of parent quest `"
            + questName + "` is invalid");
}

Result errorActionInvalidStatus(const Str& fileName, int line, int col) noexcept {
    return errorParserError(fileName, line, col, 
            "Invalid status. Expecting DONE, INACTIVE, UNREACHABLE or ACTIVE keyword.");
}


// ================================ Quest =================================== //

Result errorQuestError(const Str& serverWorldName, const Str& msg) noexcept {
    return Result::Error("([Server]:[World])(" + serverWorldName + ") " + msg);
}

Result errorQuestAlreadyExists(const Str& serverWorldName, const Str& questName) noexcept {
    return errorActionError(
            serverWorldName, "Action `" + questName + "` already exists.");
}

Result errorUndefinedQuest(const Str& serverWorldName, const Str& questName) noexcept {
    return errorQuestError(
        serverWorldName, "Quest `" + questName + "` is undefined.");
}

Result errorUndefinedSubQuest(const Str& serverWorldName, const Str& subquestName) noexcept {
    return errorQuestError(
        serverWorldName, "Subquest `" + subquestName + "` is undefined.");
}

Result errorUndefinedMainQuest(const Str& serverWorldName, const Str& questName) noexcept {
    return errorQuestError(
        serverWorldName, "Main quest `" + questName + "` is undefined.");
}

Result errorQuestCantDefine(const Str& serverWorldName, const Str& questName) noexcept {
    return errorActionError(serverWorldName, 
            "Can't define action `" + questName + "`. See the previous error.");
}

Result errorQuestPreconditionsError() noexcept {
    return Result::Error("Quest `preconditions` section error. See the previous error.");
}

Result errorQuestGoalError(const int goalIndx) noexcept {
    std::stringstream strStream;
    strStream << "Quest " << (goalIndx + 1) << "-th ";
    strStream << "`goal:` section error. See the previous error.";
    return Result::Error(strStream.str());
}

Result errorQuestActionsError() noexcept {
    return Result::Error("Quest `actions:` section error. See the previous error(s).");
}

Result errorQuestSubquestsError() noexcept {
    return Result::Error("Quest `subquests:` section error. See the previous error(s).");
}

Result errorQuestActionIsGlobal(const Str& questName, const Str& actionName) noexcept {
    return errorQuestError(questName, "Action `" + actionName + "` is global. "
            + "Only local action can be listed as a quest action.");
}


}
