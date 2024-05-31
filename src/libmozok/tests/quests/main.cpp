// Copyright 2024 Pavlo Savchuk. Subject to the MIT license.

// This tool solves complex quests that can contain subquests.
// It is the simplest possible emulation of real game-play where the server
// performs planning in a worker thread, and the user interacts with the server
// by pushing actions and reading the messages.

#include <iostream>
#include <memory>
#include <string.h>
#include <sstream>
#include <fstream>
#include <chrono>
#include <unordered_map>

#include <libmozok/mozok.hpp>
#include <utils/utils.hpp>

using namespace std;
using namespace mozok;


/// @brief Show a warning message if this much time has passed since the last 
///        message.
const auto WARNING_DURATION = chrono::milliseconds(40);

/// @brief Error if this much time has passed since the last message.
const auto ERROR_DURATION = chrono::milliseconds(300);

/// @brief Error if total running time is more than this value.
const auto ERROR_MAX_RUNTIME = chrono::milliseconds(5000);


/// @brief Quest solver message processor.
class MyMessageProcessor : public DebugMessageProcessor {
    struct QuestData {
        Str questName;
        Str parentQuestName;
        Str nextAction;
        StrVec nextActionArgs;
        QuestStatus lastStatus;
    };
    std::unordered_map<Str, QuestData> _data;
    Result _status;
    int _actionIndx;

public:
    MyMessageProcessor() noexcept 
    : _actionIndx(0)
    { /* empty */ }

    void onActionError(
            const mozok::Str& /*worldName*/, 
            const mozok::Str& /*actionName*/,
            const mozok::StrVec& /*actionArguments*/,
            const mozok::Result& errorResult
            ) noexcept override {
        _status <<= errorResult;
    }

    void onNewMainQuest(
            const Str& worldName, 
            const Str& questName
            ) noexcept override {
        DebugMessageProcessor::onNewMainQuest(worldName, questName);
        if(_data.find(questName) != _data.end()) {
            _status <<= Result::Error(
                    "QS | Quest `" + questName + "` already exist.");
            return;
        }
        _data[questName] = {questName, "", "", {}, MOZOK_QUEST_STATUS_UNKNOWN};
    }

    void onNewSubQuest(
            const Str& worldName, 
            const Str& subquestName,
            const Str& parentQuestName,
            const int goal
            ) noexcept override {
        DebugMessageProcessor::onNewSubQuest(
                worldName, subquestName, parentQuestName, goal);
        if(_data.find(subquestName) != _data.end()) {
            _status <<= Result::Error(
                    "QS | Quest `" + subquestName + "` already exist.");
            return;
        } else if(_data.find(parentQuestName) == _data.end()) {
            _status <<= Result::Error(
                    "QS | Parent quest `" + parentQuestName 
                    + "` for a subquest `" + subquestName 
                    + "` doesn't exist.");
            return;
        }
        _data[subquestName] = {
                subquestName, parentQuestName, 
                "", {}, MOZOK_QUEST_STATUS_UNKNOWN};
    }

    void onNewQuestStatus(
            const Str& worldName, 
            const Str& questName,
            const QuestStatus questStatus
            ) noexcept override {
        DebugMessageProcessor::onNewQuestStatus(
                worldName, questName, questStatus);
        if(_data.find(questName) == _data.end()) {
            _status <<= Result::Error(
                    "QS | Quest `" + questName + "` doesn't exist.");
            return;
        }
        _data[questName].lastStatus = questStatus;
    }

    void onNewQuestPlan(
            const mozok::Str& /*worldName*/, 
            const mozok::Str& questName,
            const mozok::StrVec& actionList,
            const mozok::Vector<mozok::StrVec>& actionArgsList
            ) noexcept override {
        // Uncomment this if you want to output the full plan.
        /*DebugMessageProcessor::onNewQuestPlan(
                worldName, questName, actionList, actionArgsList);*/
        if(_data.find(questName) == _data.end()) {
            _status <<= Result::Error(
                    "QS | Quest `" + questName + "` doesn't exist.");
            return;
        }
        if(actionList.size() == 0) {
            if(_data[questName].lastStatus != MOZOK_QUEST_STATUS_DONE) {
                _status <<= Result::Error(
                        "QS | Quest `" + questName 
                        + "` has an empty plan despite not being done.");
                return;
            }
            _data[questName].nextAction = "";
            _data[questName].nextActionArgs = {};
        } else {
            _data[questName].nextAction = actionList.front();
            _data[questName].nextActionArgs = actionArgsList.front();
        }
    }

    /// @return Returns `true` only if all registered quests are `DONE`.
    bool isAllQuestsDone() const {
        if(_data.size() == 0)
            return false;
        for(const auto& data : _data)
            if(data.second.lastStatus != MOZOK_QUEST_STATUS_DONE)
                return false;
        return true;
    }

    /// @brief Applies (pushes) the next applicable action.
    /// @param worldName The name of the world.
    /// @param server Quest server.
    /// @return Returns `true` if any new action was applied.
    ///         Returns `false` if no new action was applied.
    bool applyNextApplicableAction(
            const Str& worldName,
            const unique_ptr<Server>& server) {
        for(auto& data : _data) {
            if(data.second.lastStatus == MOZOK_QUEST_STATUS_UNKNOWN)
                continue;
            if(data.second.nextAction.size() == 0)
                continue;
            if(server->getActionStatus(worldName, data.second.nextAction)
                    != Server::ACTION_APPLICABLE)
                continue;
            // Next action is applicable, apply it!
            ++_actionIndx;
            cout << _actionIndx << " : " << data.second.nextAction << " ( ";
            StrVec::size_type i=0;
            for(; i < data.second.nextActionArgs.size(); ++i) {
                cout << data.second.nextActionArgs[i];
                if(i != data.second.nextActionArgs.size() - 1)
                    cout << ", ";
            }
            cout << " )" << endl;
            _status <<= server->pushAction(
                    worldName, data.second.nextAction, data.second.nextActionArgs);
            data.second.nextAction = "";
            return true;
        }
        return false;
    }

    /// @return Returns the current status of the processor.
    const Result& getStatus() const {
        return _status;
    }

} msgProcessor;


int main(int argc, char **argv) {
    // Do not truncate the test output.
    cout << "CTEST_FULL_OUTPUT" << endl;

    // Read the arguments.
    if(argc < 3) {
        cout << "Expecting: > quest_solver [quest_name] [init_action]" << endl;
        return 0;
    }
    const Str quest_name = argv[1];
    const Str init_name = argv[2];
    const Str fname = quest_name + ".quest";
    Result status = Result::OK();

    auto server = createServerFromFile("quest_solver", quest_name, fname, status);
    
    // Check the status.
    if(status.isError()) {
        // An error has occurred.
        cout << status.getDescription() << endl;
        return 0;
    }

    // Start the worker thread and push the `Init` action.
    status <<= server->startWorkerThread();
    status <<= server->pushAction(quest_name, init_name, {});

    // "Game loop" emulation.
    bool stopLoop = false;
    bool isWaiting = false;
    auto waitFrom = chrono::system_clock::now();
    auto startFrom = chrono::system_clock::now();
    bool warningWasShown = false;
    do {
        // Apply all currently applicable actions.
        while(msgProcessor.applyNextApplicableAction(quest_name, server));

        auto runTime = chrono::system_clock::now() - startFrom;
        if(runTime > ERROR_MAX_RUNTIME) {
            cout << "ERROR: Quest took to long to solve."
                 << " Total limit reached." << endl;
            break;
        }

        // Try to process the next message.
        if(server->processNextMessage(msgProcessor)) {
            // New message has been processed.
            isWaiting = false;
            continue;
        }

        if(msgProcessor.getStatus().isError())
            break;
        if(msgProcessor.isAllQuestsDone() == true)
            break;

        if(isWaiting) {
            auto waitDuration = chrono::system_clock::now() - waitFrom;
            if(waitDuration > ERROR_DURATION) {
                cout << "ERROR: No new messages for an extended period of time." 
                     << " Wait limit Reached." << endl;
                break;
            }
            if(waitDuration > WARNING_DURATION && warningWasShown == false) {
                warningWasShown = true;
                cout << "WARNING: Waiting for a new message.." << endl;
            }
        } else {
            isWaiting = true;
            waitFrom = chrono::system_clock::now();
        }
    } while(stopLoop == false);

    // Stop the worker thread and read the status of the msgProcessor.
    while(server->stopWorkerThread() == false);
    status <<= msgProcessor.getStatus();

    if(msgProcessor.isAllQuestsDone() == false)
        status <<= Result::Error("Oops. The quest wasn't completed.");

    // Explicitly delete the quests server.
    status <<= server->deleteWorld(quest_name);
    server.release();

    // Check the status.
    if(status.isError()) {
        // An error has occurred.
        cout << status.getDescription() << endl;
        return 0;
    }

    cout << "MOZOK_OK" << endl;
    return 0;
}
