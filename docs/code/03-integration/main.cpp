// This is a minimal example of a simple game that uses LibMozok.
// It loads and initializes the world described in `main.qsf` and 
// other `.quest` files. Then, it prints the list of objects and actions.
// It reads input commands, parses them, and pushes them as actions to the server.
// It also reacts to server events such as `onActionError`, `onNewMainQuest`,
// and `onNewQuestStatus`, etc.

#include <libmozok/mozok.hpp>

#include <fstream>
#include <iterator>
#include <iostream>
#include <memory>
#include <string>
#include <sstream>

using namespace mozok;
using namespace std;


// Here we define how to read text files using paths specified in `main.qsf`.
class MyFileSystem : public FileSystem {
public:
    Result getTextFile(const Str& path, Str& out) noexcept override {
        std::ifstream file(path);
        if (file.is_open()) {
            std::string src = std::string(
                    std::istreambuf_iterator<char>(file),
                    std::istreambuf_iterator<char>());
            file.close();
            out = src;
            return Result::OK();
        }
        return Result::Error("MyFileSystem: Can't open `" + path +"` file.");
    }
};


// Here we define how to handle various server events.
// For more information, see <libmozok/message_processor.hpp>.
class MyMessageProcessor : public MessageProcessor {
public:
    void onActionError(
            const Str& /*worldName*/, 
            const Str& actionName,
            const StrVec& actionArguments,
            const Result& errorResult,
            const ActionError /*actionError*/,
            const int /*data*/
            ) noexcept override {
        cout << "* onActionError: " << actionName << "(";
        for(StrVec::size_type i = 0; i < actionArguments.size(); ++i) {
            cout << actionArguments[i];
            cout << (i < actionArguments.size() - 1 ? "," : "");
        }
        cout << ")" << endl;
        cout << "\tError Msg: " << errorResult.getDescription() << endl;
    }

    void onNewMainQuest(
            const Str& /*worldName*/, 
            const Str& questName
            ) noexcept override {
        cout << "* onNewMainQuest: " << questName << endl;
    }
    
    void onNewSubQuest(
            const Str& /*worldName*/, 
            const Str& subquestName,
            const Str& /*parentQuestName*/,
            const int /*goal*/
            ) noexcept override {
        cout << "* onNewSubQuest: " << subquestName << endl;
    }
    
    void onNewQuestStatus(
            const Str& /*worldName*/, 
            const Str& questName,
            const QuestStatus questStatus
            ) noexcept override {
        cout << "* onNewQuestStatus: " << questName << " ";
        cout << mozok::questStatusToStr_Short(questStatus) << endl;
    }

    // ...
};


int main() {
    const int ERR_CODE = 1;
    const Str SERVER_NAME = "MyServer";
    const Str MAIN_QSF = "main.qsf";
    const Str WORLD_NAME = "game";

    MyMessageProcessor message_processor;
    MyFileSystem filesystem;
    Result status;

    // Here we create LibMozok Server.
    unique_ptr<Server> server(Server::createServer(SERVER_NAME, status));

    // Here we create and initialize the quest world(s) using `main.qsf`.
    Str main_qsf;
    status <<= filesystem.getTextFile(MAIN_QSF, main_qsf);
    status <<= server->loadQuestScriptFile(&filesystem, MAIN_QSF, main_qsf, true);

    // Here we check for initialization errors.
    if(status.isError()) {
        cout << status.getDescription() << endl;
        return ERR_CODE;
    }

    // Here we start the worker thread.
    server->startWorkerThread();

    cout << "Enter :q to quit the game." << endl;
    cout << "Enter :p to skip and process the messages." << endl;
    
    // Here we print the lists of objects and actions.
    cout << "\nObjects:" << endl;
    for(const auto& obj : server->getObjects(WORLD_NAME))
        cout << "\t" << obj << endl;
    cout << "\nActions:" << endl;
    for(const auto& action : server->getActions(WORLD_NAME)) {
        cout << "\t" << action << "(";
        const auto args = server->getActionType(WORLD_NAME, action);
        for(size_t i = 0; i < args.size(); ++i)
            cout << args[i][0] << (i < args.size()-1 ? "," : "");
        cout << ")" << endl;
    }

    cout << endl;
    
    // This is the main game loop.
    // Note that planning is handled in a separate thread by LibMozok.
    while(true) {
        // Here we process all the new messages from the server.
        while(server->processNextMessage(message_processor));

        // Here we read the next command.

        cout << "next action > ";
        Str line;
        getline(cin, line);

        if(line == ":q")
            break;
        if(line == ":p")
            continue;

        size_t open_par = line.find('(');
        size_t close_par = line.find(')');

        if (open_par==Str::npos || close_par==Str::npos || close_par<open_par) {
            cout << "Invalid format" << endl;
            continue;
        }

        // Here we parse the command into an action and arguments.
        StrVec actionArgs; 
        Str arg, actionName = line.substr(0, open_par);
        stringstream ss(line.substr(open_par + 1, close_par - open_par - 1));
        while(std::getline(ss, arg, ','))
            if(arg.empty() == false)
                actionArgs.push_back(arg);

        // Here we push the action along with its arguments.
        server->pushAction(
                WORLD_NAME, // world name
                actionName, // action name
                actionArgs, // action arguments (StrVec of object names)
                0 // int code attached to this action (passed to the `onActionError`)
                );
    }

    // Here we shut down the worker thread.
    while(server->stopWorkerThread() == false);
    return 0;
}
