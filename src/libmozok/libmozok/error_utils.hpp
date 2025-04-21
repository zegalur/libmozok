// Copyright 2024 Pavlo Savchuk. Subject to the MIT license.

#pragma once

#include <libmozok/result.hpp>

namespace mozok {

// Common
Result errorNotImplemented(const char* file, int line, const char* func) noexcept;

// Parser
Result errorParserError(const Str& fileName, int line, int col, const Str& msg) noexcept;
Result errorParserWorldError(const Str& fileName, int line, int col, const Str& serverWorldName) noexcept;
Result errorParserUnsupportedVersion(const Str& fileName, int line, int col, int curMajor, int curMinor, int badMajor, int badMinor) noexcept;
Result errorExpectingKeyword(const Str& fileName, int line, int col, const Str& keyword) noexcept;
Result errorExpectingSpace(const Str& fileName, int line, int col) noexcept;
Result errorExpectingNewLine(const Str& fileName, int line, int col) noexcept;
Result errorExpectingDigit(const Str& fileName, int line, int col) noexcept;
Result errorExpectingUppercase(const Str& fileName, int line, int col) noexcept;
Result errorExpectingLowercase(const Str& fileName, int line, int col) noexcept;
Result errorExpectingUnderscore(const Str& fileName, int line, int col) noexcept;
Result errorExpectingLetter(const Str& fileName, int line, int col) noexcept;
Result errorExpectingColon(const Str& fileName, int line, int col) noexcept;
Result errorExpectingComma(const Str& fileName, int line, int col) noexcept;
Result errorInvalidKeyword(const Str& fileName, int line, int col, const Str& keyword) noexcept;
Result errorExpectingOpenPar(const Str& fileName, int line, int col) noexcept;
Result errorExpectingClosePar(const Str& fileName, int line, int col) noexcept;
Result errorExpectingOpenBracket(const Str& fileName, int line, int col) noexcept;
Result errorExpectingCloseBracket(const Str& fileName, int line, int col) noexcept;

// Server
Result errorServerWorkerIsRunning(const Str& serverName) noexcept;

// World
Result errorWorldAlreadyExists(const Str& serverName, const Str& worldName) noexcept;
Result errorWorldDoesntExist(const Str& serverName, const Str& worldName) noexcept;
Result errorWorldOtherError(const Str& serverWorldName, const Str& msg) noexcept;
Result errorCantApplyNAAction(const Str& serverWorldName, const Str& actionName) noexcept;

// Type
Result errorTypeAlreadyExists(const Str& serverWorldName, const Str& typeName) noexcept;
Result errorUndefinedType(const Str& serverWorldName, const Str& typeName) noexcept;
Result errorTypeCantDefine(const Str& serverWorldName, const Str& typeName) noexcept;

// Object
Result errorObjectAlreadyExists(const Str& serverWorldName, const Str& objectName) noexcept;
Result errorUndefinedObject(const Str& serverWorldName, const Str& objectName) noexcept;
Result errorObjectCantDefine(const Str& serverWorldName, const Str& objectName) noexcept;

// Relation
Result errorRelAlreadyExists(const Str& serverWorldName, const Str& relationName) noexcept;
Result errorRelationCantDefine(const Str& serverWorldName, const Str& relationName) noexcept;
Result errorUndefinedRel(const Str& serverWorldName, const Str& relationName) noexcept;
Result errorRelArgError_InvalidArity(const Str& relationName, const int expectedArity, const int givenArity) noexcept;
Result errorRelArgError_InvalidType(const Str& relationName, const int indx, const Str& argObjName, const StrVec& argObjType, const Str& expectedType) noexcept;

// Relation List
Result errorRListAlreadyExists(const Str& serverWorldName, const Str& relationListName) noexcept;
Result errorRListCantDefine(const Str& serverWorldName, const Str& relationListName) noexcept;
Result errorUndefinedRList(const Str& serverWorldName, const Str& relationListName) noexcept;
Result errorRListArgError_InvalidArity(const Str& rlistName, const int expectedArity, const int givenArity) noexcept;
Result errorRListArgError_InvalidType(const Str& rlistName, const int indx, const Str& argObjName, const StrVec& argObjType, const StrVec& expectedType) noexcept;

// Action
Result errorActionAlreadyExists(const Str& serverWorldName, const Str& actionName) noexcept;
Result errorActionCantDefine(const Str& serverWorldName, const Str& actionName) noexcept;
Result errorUndefinedAction(const Str& serverWorldName, const Str& actionName) noexcept;
Result errorActionPreError() noexcept;
Result errorActionRemError() noexcept;
Result errorActionAddError() noexcept;
Result errorActionArgError_InvalidArity(const Str& actionName, const int expectedArity, const int givenArity) noexcept;
Result errorActionArgError_InvalidType(const Str& actionName, const int indx, const Str& argObjName, const StrVec& argObjType, const StrVec& expectedType) noexcept;
Result errorActionPreconditionsFailed(const Str& serverWorldName, const Str& actionName) noexcept;
Result errorActionSetStatusGoalError(const Str& serverWorldName, const Str& actionName, const Str& questName, const int errorGoalIndx) noexcept;
Result errorActionSetStatusParentGoalError(const Str& serverWorldName, const Str& actionName, const Str& questName, const int errorGoalIndx) noexcept;
Result errorActionInvalidStatus(const Str& fileName, int line, int col) noexcept;

// Quest
Result errorQuestAlreadyExists(const Str& serverWorldName, const Str& actionName) noexcept;
Result errorQuestCantDefine(const Str& serverWorldName, const Str& questName) noexcept;
Result errorUndefinedQuest(const Str& serverWorldName, const Str& questName) noexcept;
Result errorUndefinedSubQuest(const Str& serverWorldName, const Str& subquestName) noexcept;
Result errorUndefinedMainQuest(const Str& serverWorldName, const Str& questName) noexcept;
Result errorQuestPreconditionsError() noexcept;
Result errorQuestGoalError(const int goalIndx) noexcept;
Result errorQuestActionsError() noexcept;
Result errorQuestSubquestsError() noexcept;
Result errorQuestActionIsGlobal(const Str& questName, const Str& actionName) noexcept;

}
