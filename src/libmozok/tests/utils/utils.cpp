// Copyright 2024 Pavlo Savchuk. Subject to the MIT license.

#include <utils/utils.hpp>

#include <iostream>
#include <memory>
#include <string.h>
#include <sstream>
#include <fstream>
#include <chrono>

using namespace std;
using namespace mozok;


void DebugMessageProcessor::onActionError(
        const Str&, 
        const Str& /*actionName*/,
        const StrVec& /*actionArguments*/,
        const Result& errorResult,
        const mozok::ActionError actionError,
        const int data
        ) noexcept {
    cout << "> Action error: " << errorResult.getDescription() << endl;
}

void DebugMessageProcessor::onNewMainQuest(
        const Str&, 
        const Str& questName) noexcept {
    cout << "> New main quest: " << questName << endl;
}

void DebugMessageProcessor::onNewSubQuest(
        const Str& , 
        const Str& subquestName,
        const Str& parentQuestName,
        const int goal
        ) noexcept {
    cout << "> New subquest: " << subquestName 
         << ". Parent quest: " << parentQuestName 
         << ". Goal = " << goal << endl;
}

void DebugMessageProcessor::onNewQuestStatus(
        const Str&, 
        const Str& questName,
        const QuestStatus questStatus
        ) noexcept {
    cout << "> New quest status: " << questName 
         << " = " << questStatusToStr(questStatus) 
         << endl;
}

void DebugMessageProcessor::onNewQuestGoal(
        const Str&,
        const Str& questName,
        const int newGoal,
        const int oldGoal
        ) noexcept {
    cout << "> New quest goal: " << questName 
         << " " << oldGoal << " -> " << newGoal
         << endl;
}


void DebugMessageProcessor::onNewQuestPlan(
        const Str&, 
        const Str& questName,
        const StrVec& actionList,
        const Vector<StrVec>& actionArgsList
        ) noexcept {
    cout << "> New quest plan: " << questName << endl;
    for(StrVec::size_type i=0; i<actionList.size(); ++i) {
        cout << "  " << (i+1) << ". " << actionList[i] << " ( ";
        for(StrVec::size_type j=0; j<actionArgsList[i].size(); ++j)
            cout << actionArgsList[i][j] 
                << (j != actionArgsList[i].size()-1 ? ", " : "");
        cout << " )" << endl;
    }
}

void DebugMessageProcessor::onSearchLimitReached(
        const mozok::Str&,
        const mozok::Str& questName,
        const int searchLimitValue
        ) noexcept {
    cout << "> Search limit " << searchLimitValue 
         << " reached for `" << questName << "`" << endl;
}

void DebugMessageProcessor::onSpaceLimitReached(
        const mozok::Str&,
        const mozok::Str& questName,
        const int spaceLimitValue
        ) noexcept {
    cout << "> Space limit " << spaceLimitValue 
         << " reached for `" << questName << "`" << endl;
}


unique_ptr<Server> createServerFromFile(
        const Str& serverName,
        const Str& worldName,
        const Str& fileName,
        Result& out) {
    // Open the puzzle project file.
    ifstream project_file(fileName.c_str());
    if(project_file.fail()) {
        out <<= Result::Error(
                "File error. File not found or other I/O related error.");
        return nullptr;
    }
    
    // Read the .quest file.
    stringstream project_sstream;
    project_sstream << project_file.rdbuf();

    // Create a new quests server.
    unique_ptr<Server> server(Server::createServer(serverName, out));
    if(out.isError())
        return nullptr;

    // Create puzzle quest world.
    out <<= server->createWorld(worldName);

    // Add the project to the world.
    out <<= server->addProject(worldName, fileName, project_sstream.str());
    return server;
}
