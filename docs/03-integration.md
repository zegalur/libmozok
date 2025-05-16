# 3. Engine Integration

In this lesson, we’ll show how to integrate LibMozok into your own game engine. You’ll learn how to set up the server, handle player actions, process messages etc.

All files used in this tutorial are available in the `docs/code/03-integration` directory.

## Table of Contents

- [Before We Start](#before-we-start)
- [Overview](#overview)
- [Minimal Working Example](#minimal-working-example)
- [Simplest Game](#simplest-game)
- [Code Generation](#code-generation)
- [Further Reading](#further-reading)

## Before We Start

Download and install LibMozok as described in the [README.md](../README.md#installation). You’ll need the Mozok libraries and headers available in `<repo>/install`. For a better development experience, we recommend installing syntax highlighting plugins for (Neo)Vim or VSCode.

Also, make sure you’ve completed the two previous tutorials.

## Overview

LibMozok is a relatively small C++ library designed to be used in real-time games. It utilizes an additional thread for planning, so it runs in parallel and doesn’t impact the game’s FPS. It has a small, clean public interface, making it easy to integrate with any engine or game. All symbols are defined inside the `mozok` namespace. The library does not use exceptions for error handling.

List of key headers:

* `<libmozok/mozok.hpp>` – Main header. Include this to access all core features.
* `<libmozok/public_types.hpp>` – Defines type aliases for strings (`Str`), string arrays (`StrVec`), and vectors (`Vector`). When using the LibMozok API, prefer these types over raw STL types like `std::string`.
* `<libmozok/result.hpp>` – Defines the `Result` type used for error handling. A result can be either "OK" or "Error" (`result.isOk()` and `result.isError()`). If it’s an error, you can retrieve the error message using `result.getDescription()`. You can also chain results using `result_1 <<= result_2`. See the comments in `result.hpp` for more information.
* `<libmozok/server.hpp>` – Defines `Server`, the core class of the LibMozok API.
* `<libmozok/filesystem.hpp>` – Defines the `FileSystem` interface used for file access.
* `<libmozok/message_processor.hpp>` – Derive from this class to handle messages from the Mozok server.

A typical usage flow looks like this:

1. Define your own `FileSystem` and `MessageProcessor` classes for file access and message handling.
2. Create a `Server` instance.
3. Create and initialize one or more quest worlds by loading a `.qsf` file or building them via the server’s API.
4. Start the worker thread.
5. Inside your game loop:
    1. Push player actions to the server.
    2. Process messages returned by the server.
6. To save the game state:
    1. Pause the game.
    2. Flush the server state (stop the worker thread and process messages, repeat until there are no more messages and/or actions left).
    3. Save the state of each quest world.
    4. Restart the worker thread.
    5. Resume the game.
7. When done, stop the worker thread.
8. Delete the server instance.

## Minimal Working Example

This code is the minimal working example: 
```c++
#include <libmozok/mozok.hpp>

// ...

using namespace mozok;

// ...

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
    void onNewMainQuest(
            const Str& /*worldName*/, 
            const Str& questName
            ) noexcept override {
        cout << "* onNewMainQuest: " << questName << endl;
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

    // ...
    
    // This is the main game loop.
    // Note that planning is handled in a separate thread by LibMozok.
    while(true) {
        // Here we process all the new messages from the server.
        while(server->processNextMessage(message_processor));

        // ...

        // Here we push the action along with its arguments.
        server->pushAction(
                WORLD_NAME, // world name
                "ActionName", // action name
                {"arg_1_", "arg_2_"}, // action arguments (StrVec of object names)
                0 // int code attached to this action (passed to the `onActionError`)
                );
    }

    // ...

    // Here we shut down the worker thread.
    while(server->stopWorkerThread() == false);
    return 0;
}
```

## Simplest Game

This code is a detailed version of the previous example:

```c++
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
```

This is an example of how the game could progress using quests from the previous tutorial:

```
Enter :q to quit the game.
Enter :p to skip and process the messages.

Objects:
        john_
        anne_
        thief_
        old_man_
        home_
        forest_
        hospital_
        market_
        antidote_
        antidote_2_
        wrong_antidote_

Actions:
        Init()
        TakeInfected(player,character,from,to)
        SaveInfected(player,character,safe_place)
        Protect(player,wife,place)
        GoTo(player,from,to)
        Take(player,item,place)
        Heal(player,antidote,place,character)
        StealItem(character,item,place)
        BuyItem(player,character,item,place)
        SwitchItem(character,item_1,item_2)
        KillInfected(player,character,place)
        Escape(player,wife)

* onNewMainQuest: SaveFamily
* onNewQuestStatus: SaveFamily REACHABLE
next action > TakeInfected(john_,anne_,forest_,home_)
next action > :p
* onNewSubQuest: SaveWife
* onNewQuestStatus: SaveWife REACHABLE
next action > GoTo(john_,home_,hospital_)
next action > Take(john_,antidote_,hospital_)
next action > GoTo(john_,hospital_,home_)
next action > Heal(john_,antidote_,home_,anne_)
next action > :p
* onNewQuestStatus: SaveWife DONE
next action > Protect(john_,anne_,home_)
next action > :p
* onNewQuestStatus: SaveFamily DONE
next action > :q
```
## Code Generation

Instead of just using string literals, consider auto-generating a C++ header using the server’s `getWorlds()`, `getObjects()`, `getActions()`, `getActionType()` etc. server methods.

A working example of GDScript auto-generation can be found in the libmozok-godot demo.  
Specifically, see `<libmozok-godot>/demo/quests/generated/` for the generated code,  
and `<libmozok-godot>/demo/addons/mozok_utils/utils_screen.gd` for the generator implementation.

## Further Reading

[Further Reading List](../README.md#further-reading)
