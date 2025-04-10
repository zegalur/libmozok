// Copyright 2024 Pavlo Savchuk. Subject to the MIT license.

// The puzzle solver automatically solves a puzzle described in a given .quest 
// file. The puzzle's .quest file must contain only one main quest without 
// subquests. If the given .quest file contains no errors and the puzzle has 
// been solved, the puzzle solver will output MOZOK_OK at the end.

#include <iostream>
#include <chrono>

#include <libmozok/mozok.hpp>
#include <utils/utils.hpp>

using namespace std;
using namespace mozok;


/// @brief Puzzle solver message processor.
class MyMessageProcessor : public DebugMessageProcessor {
    /// @brief If a puzzle plan was found, this field will contain the list of 
    ///        plan action names.
    StrVec _puzzleActionList;

    /// @brief If a puzzle plan was found, this field will contain the list of 
    ///        plan action argument names.
    Vector<StrVec> _puzzleActionArguments;

    /// @brief Is puzzle quest is reachable or done.
    bool _isReachableOrDone;

    /// @brief Is puzzle quest is done.
    bool _isDone;

public:
    MyMessageProcessor() noexcept :
        _puzzleActionList(),
        _puzzleActionArguments(),
        _isReachableOrDone(false),
        _isDone(false)
    { /* empty */ }

    const StrVec& getPuzzleActions() const noexcept 
        { return _puzzleActionList; }
    const Vector<StrVec>& getPuzzleActionArguments() const noexcept 
        { return _puzzleActionArguments; }
    bool isReachableOrDone() const noexcept 
        { return _isReachableOrDone; }
    bool isDone() const noexcept 
        { return _isDone; }

    void onNewQuestStatus(
            const Str&, 
            const Str& questName,
            const QuestStatus questStatus
            ) noexcept override {
        _isDone = (questStatus == MOZOK_QUEST_STATUS_DONE);
        _isReachableOrDone = (questStatus == MOZOK_QUEST_STATUS_REACHABLE);
        _isReachableOrDone = _isReachableOrDone || _isDone;
        DebugMessageProcessor::onNewQuestStatus("", questName, questStatus);
    }

    void onNewQuestPlan(
            const Str&, 
            const Str& questName,
            const StrVec& actionList,
            const Vector<StrVec>& actionArgsList
            ) noexcept override {
        _puzzleActionList = actionList;
        _puzzleActionArguments = actionArgsList;
        DebugMessageProcessor::onNewQuestPlan(
                "", questName, actionList, actionArgsList);
    }

} msgProcessor;


/// @brief Performs a planning command.
/// @param server A server where planning will be conducted.
void performPlanning(unique_ptr<Server> &server) noexcept {
    chrono::steady_clock::time_point begin = chrono::steady_clock::now();
    server->performPlanning();
    chrono::steady_clock::time_point end = chrono::steady_clock::now();
    cout << "Planning elapsed time = " 
         << chrono::duration_cast<chrono::microseconds>(end - begin).count() 
         << "[µs]" << std::endl;
}


int main(int argc, char **argv) {
    // Do not truncate the test output.
    cout << "CTEST_FULL_OUTPUT" << endl;

    // Read the arguments.
    if(argc < 3) {
        cout << "Expecting: > puzzle_solver [puzzle_name] [init_action]" 
             << endl;
        return 0;
    }
    const Str puzzle_name = argv[1];
    const Str init_action = argv[2];
    const Str fname = puzzle_name + ".quest";
    Result status = Result::OK();

    auto server = createServerFromFile("puzzle_solver", puzzle_name, fname, status);
    if(status.isError()) {
        cout << status.getDescription() << endl;
        return 0;
    }

    // Initialize the puzzle.
    // Puzzle project must contain Init() action.
    status <<= server->applyAction(puzzle_name, init_action, {});
    
    if(status.isError()) {
        cout << status.getDescription() << endl;
        return 0;
    }

    // Perform planning and read all the messages.
    performPlanning(server);
    while(server->processNextMessage(msgProcessor));

    // Check if puzzle is solvable.
    if(msgProcessor.isReachableOrDone() == false) {
        cout << "error: Puzzle is inactive, unreachable or with unknown status.";
        cout << endl;
        return 0;
    }

    // Solve the puzzle by applying the plan actions.
    const auto* puzzleActions = &(msgProcessor.getPuzzleActions());
    const auto* puzzleArgs = &(msgProcessor.getPuzzleActionArguments());
    for(StrVec::size_type i = 0; i < puzzleActions->size(); ++i) {
        const Str& actionName = puzzleActions->at(i);
        const StrVec& arguments = puzzleArgs->at(i);
        status <<= server->applyAction(puzzle_name, actionName, arguments);
    }

    if(status.isError()) {
        cout << status.getDescription() << endl;
        return 0;
    }

    // Perform planning and read all the messages.
    // At this point main quest must be already done.
    server->performPlanning();
    while(server->processNextMessage(msgProcessor));

    // Generate a savefile.
    const Str saveFile = server->generateSaveFile(puzzle_name);
    cout << "\nSave file:" << endl;
    cout << saveFile << endl;
    cout << "END OF SAVE FILE" << endl;

    // Check if puzzle is solved.
    if(msgProcessor.isDone() == false) {
        cout << "error: Puzzle isn't solved after applying the plan actions.";
        cout << endl;
        return 0;
    } else 
        cout << "\nPuzzle solved!\n" << endl;

    // Explicitly delete the quests server.
    status <<= server->deleteWorld(puzzle_name);
    server.release();

    if(status.isError()) {
        cout << status.getDescription() << endl;
        return 0;
    }

    cout << "MOZOK_OK" << endl;

    return 0;
}
