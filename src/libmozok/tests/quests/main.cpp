// Copyright 2024-2025 Pavlo Savchuk. Subject to the MIT license.

// This tool solves complex quests that can contain subquests.
// It is the simplest possible emulation of real game-play where the server
// performs planning in a worker thread, and the user interacts with the server
// by pushing actions and reading the messages.

#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include <sstream>
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
class QuestSolver : public DebugMessageProcessor {

    struct Quest;
    using QuestPtr = std::shared_ptr<Quest>;

    /// @brief Represents an opened quest.
    struct Quest {
        /// @brief Quest name.
        Str name;
        /// @brief Parent quest name.
        Str parent;
        /// @brief Currently opened subquest names.
        StrVec subquests;
        /// @brief Last received status from the server.
        QuestStatus lastStatus;
        /// @brief Last received goal.
        int goalIndx;
        /// @brief List of actions from the first received plan
        StrVec actionList;
        /// @brief List of arguments from the first received plan.
        Vector<StrVec> actionArgsList;
        /// @brief Index of the next action that wait for been applied.
        int nextAction;
        /// @brief How many N/A actions have been skipped.
        int skipped;
    };

    Vector<Str> _mainQuests;
    std::unordered_map<Str, QuestPtr> _quests;

    /// @brief Count how many actions was applied in total by the server.
    int _actionIndx;

    /// @brief Quest solver status.
    Result _status;

public:
    QuestSolver() noexcept 
    : _actionIndx(0)
    { /* empty */ }

    void onActionError(
            const Str& /*worldName*/, 
            const Str& /*actionName*/,
            const StrVec& /*actionArguments*/,
            const Result& errorResult,
            const ActionError /*actionError*/,
            const int /*data*/
            ) noexcept override {
        _status <<= errorResult;
    }

    void onNewMainQuest(
            const Str& worldName, 
            const Str& questName
            ) noexcept override {
        DebugMessageProcessor::onNewMainQuest(worldName, questName);
        QuestPtr q = std::make_shared<Quest>();
        q->lastStatus = QuestStatus::MOZOK_QUEST_STATUS_UNKNOWN;
        q->name = questName;
        q->parent = "";
        q->goalIndx = -1;
        q->nextAction = -1;
        q->skipped = 0;
        _quests[questName] = q;
        _mainQuests.push_back(questName);
    }

    void onNewSubQuest(
            const Str& worldName, 
            const Str& subquestName,
            const Str& parentQuestName,
            const int goal
            ) noexcept override {
        DebugMessageProcessor::onNewSubQuest(
                worldName, subquestName, parentQuestName, goal);
        if(_quests.find(subquestName) != _quests.end()) {
            _status <<= Result::Error(
                    "QS | Quest `" + subquestName + "` already exist.");
            return;
        } else if(_quests.find(parentQuestName) == _quests.end()) {
            _status <<= Result::Error(
                    "QS | Parent quest `" + parentQuestName 
                    + "` for a subquest `" + subquestName 
                    + "` doesn't exist.");
            return;
        }
        QuestPtr q = std::make_shared<Quest>();
        q->lastStatus = QuestStatus::MOZOK_QUEST_STATUS_UNKNOWN;
        q->name = subquestName;
        q->parent = parentQuestName;
        q->goalIndx = -1;
        q->nextAction = -1;
        q->skipped = 0;
        _quests[subquestName] = q;
        _quests[q->parent]->subquests.push_back(subquestName);
    }

    void onNewQuestStatus(
            const Str& worldName, 
            const Str& questName,
            const QuestStatus questStatus
            ) noexcept override {
        DebugMessageProcessor::onNewQuestStatus(
                worldName, questName, questStatus);
        if(_quests.find(questName) == _quests.end()) {
            _status <<= Result::Error(
                    "QS | Quest `" + questName + "` doesn't exist.");
            return;
        }
        _quests[questName]->lastStatus = questStatus;
    }

    void onNewQuestPlan(
            const Str& /*worldName*/, 
            const Str& questName,
            const StrVec& actionList,
            const Vector<StrVec>& actionArgsList
            ) noexcept override {
        // Uncomment this if you want to output the full plan.
        /*DebugMessageProcessor::onNewQuestPlan(
                worldName, questName, actionList, actionArgsList);*/
        if(_quests.find(questName) == _quests.end()) {
            _status <<= Result::Error(
                    "QS | Quest `" + questName 
                    + "` doesn't exist." + " (onNewQuestPlan)");
            return;
        }
        if(actionList.size() != actionArgsList.size()) {
            _status <<= Result::Error(
                    "QS | actionList.size() != actionArgsList.size()");
            return;
        }
        if(actionList.size() == 0)
            if(_quests[questName]->lastStatus != MOZOK_QUEST_STATUS_DONE) {
                _status <<= Result::Error(
                        "QS | Quest `" + questName 
                        + "` has an empty plan despite not being done.");
                return;
            }
        // Only the first plan is used for each quest.
        if(_quests[questName]->nextAction == -1) {
            _quests[questName]->nextAction = 0;
            _quests[questName]->actionList = actionList;
            _quests[questName]->actionArgsList = actionArgsList;
        }
    }

    /// @return Returns `true` only if all registered quests are `DONE`.
    bool isAllQuestsDone() const {
        if(_quests.size() == 0)
            return false;
        for(const auto& q : _quests)
            if(q.second->lastStatus != MOZOK_QUEST_STATUS_DONE)
                return false;
        return true;
    }

    /// @brief Applies (pushes) the next applicable action of a quest tree.
    /// @param worldName The name of the world.
    /// @param server Quest server.
    /// @param quest Quest data.
    /// @return Returns `true` if any new action was applied.
    ///         Returns `false` if no new action was applied.
    bool applyNext(
            const Str& worldName,
            const unique_ptr<Server>& server,
            QuestPtr& quest) {
        if(quest->lastStatus == MOZOK_QUEST_STATUS_DONE)
            return false;
        if(quest->lastStatus == MOZOK_QUEST_STATUS_UNREACHABLE)
            return false;
        if(quest->nextAction < 0)
            return false;
        bool allSubsDone = true;
        for(const auto& sub : quest->subquests) {
            if(applyNext(worldName, server, _quests[sub]))
                return true;
            allSubsDone &= _quests[sub]->lastStatus == MOZOK_QUEST_STATUS_DONE;
        }
        if(allSubsDone == false) 
            return false;

        // All current subquests are DONE.
        
        while(size_t(quest->nextAction) < quest->actionList.size()) {
            const Str &actionName = quest->actionList[quest->nextAction];
            
            // Check if this action is N/A.
            if(server->getActionStatus(
                        worldName, quest->actionList[quest->nextAction])
                    != Server::ACTION_APPLICABLE) {
                // Skip N/A action only when corresponding subquest is DONE.
                if(quest->subquests.size() > StrVec::size_type(quest->skipped)) {
                    ++quest->nextAction;
                    ++quest->skipped;
                    return true;
                }
                return false;
            }

            // This action is applicable.
            ++_actionIndx;
            const StrVec &args = quest->actionArgsList[quest->nextAction];
            ++quest->nextAction;

            // Show the message.
            cout << _actionIndx << " : " << actionName << " ( ";
            StrVec::size_type i=0;
            for(; i < args.size(); ++i) {
                cout << args[i];
                if(i != args.size() - 1)
                    cout << ", ";
            }
            cout << " )" << endl;

            // Push the action into a queue.
            ActionError actionError = MOZOK_AE_NO_ERROR;
            _status <<= server->pushAction(
                    worldName, actionName, args, actionError);
            return true;
        }
        return false;
    } 

    /// @brief Applies (pushes) the next applicable action.
    /// @param worldName The name of the world.
    /// @param server Quest server.
    /// @return Returns `true` if any new action was applied.
    ///         Returns `false` if no new action was applied.
    bool applyNextApplicableAction(
            const Str& worldName,
            const unique_ptr<Server>& server) {
        for(auto& mainQuestName : _mainQuests)
            if(applyNext(worldName, server, _quests[mainQuestName]))
                return true;
        return false;
    }

    /// @return Returns the current status of the processor.
    const Result& getStatus() const {
        return _status;
    }

} questSolver;


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
    ActionError actionError = MOZOK_AE_NO_ERROR;
    status <<= server->startWorkerThread();
    status <<= server->pushAction(quest_name, init_name, {}, actionError);

    // "Game loop" emulation.
    bool stopLoop = false;
    bool isWaiting = false;
    auto waitFrom = chrono::system_clock::now();
    auto startFrom = chrono::system_clock::now();
    bool warningWasShown = false;
    do {
        // Apply all currently applicable actions.
        while(questSolver.applyNextApplicableAction(quest_name, server));

        auto runTime = chrono::system_clock::now() - startFrom;
        if(runTime > ERROR_MAX_RUNTIME) {
            cout << "ERROR: Quest took to long to solve."
                 << " Total limit reached." << endl;
            break;
        }

        // Try to process the next message.
        if(server->processNextMessage(questSolver)) {
            // New message has been processed.
            isWaiting = false;
            continue;
        }

        if(questSolver.getStatus().isError())
            break;
        if(questSolver.isAllQuestsDone() == true)
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

    // Stop the worker thread and read the status of the questSolver.
    while(server->stopWorkerThread() == false);
    status <<= questSolver.getStatus();

    if(questSolver.isAllQuestsDone() == false)
        status <<= Result::Error("Oops. The quest wasn't completed.");

    // Generate the first savefile.
    const Str saveFile = server->generateSaveFile(quest_name);
    cout << "\nSave file #1:" << endl << saveFile << endl;
    cout << "END OF SAVE FILE #1\n" << endl;

    // Explicitly delete the first quests server.
    status <<= server->deleteWorld(quest_name);
    server.reset();

    // Check the status.
    if(status.isError()) {
        // An error has occurred.
        cout << status.getDescription() << endl;
        return 0;
    }

    // Testing the `generateSaveFile()` method.

    cout << endl;
    cout << "LOADING..." << endl;

    auto server2 = createServerFromFile("quest_solver", quest_name, fname, status);

    // Load the state from the generated save file.
    status <<= server2->addProject(quest_name, "saveFile", saveFile);
    server2->applyAction(quest_name, "Load", {}, actionError);
    server2->performPlanning();

    // Process all the messages.
    DebugMessageProcessor debugMsgProcessor;
    while(server2->processNextMessage(debugMsgProcessor));

    // Generate the second save file.
    const Str saveFile2 = server2->generateSaveFile(quest_name);

    // Delete the second server.
    status <<= server2->deleteWorld(quest_name);
    server2.reset();

    // Check the current status.
    if(status.isError()) {
        // An error has occurred.
        cout << status.getDescription() << endl;
        return 0;
    }

    // Now, check if two save files have identical states.
    // The states are considered identical if every line from the first file
    // is present in the second file and vice versa.
    stringstream firstSaveFile(saveFile);
    stringstream secondSaveFile(saveFile2);
    Str textLine;
    bool hasSaveFileError = false;
    while(getline(firstSaveFile, textLine, '\n')) {
        if(saveFile2.find(textLine) == Str::npos) {
            cout << "SAVE_FILE_ERROR: ";
            cout << "A text line from the first save file is not present in ";
            cout << "the second save file." << endl;
            cout << "Text line: `" << textLine << "`." << endl;
            hasSaveFileError = true;
        }
    }
    while(getline(secondSaveFile, textLine, '\n')) {
        if(saveFile.find(textLine) == Str::npos) {
            cout << "SAVE_FILE_ERROR: ";
            cout << "A text line from the second save file is not present in ";
            cout << "the first save file." << endl;
            cout << "Text line: `" << textLine << "`." << endl;
            hasSaveFileError = true;
        }
    }
    if(hasSaveFileError) {
        cout << "\nSave file #2:" << endl << saveFile2 << endl;
        cout << "END OF SAVE FILE #2\n" << endl;
        return 0;
    }

    // Save files have identical states.
    cout << "\nSave files have identical states.\n" << endl;

    cout << "MOZOK_OK" << endl;
    return 0;
}
