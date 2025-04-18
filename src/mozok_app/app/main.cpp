// Copyright 2025 Pavlo Savchuk. Subject to the MIT license.

// LibMozok's developing and debugging tool for quests.
// For more information please read `/docs/mozok_app.md`

#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <sstream>
#include <libmozok/mozok.hpp>
#include <unordered_map>

#include "app/app.hpp"
#include "app/script.hpp"
#include "app/strings.hpp"
#include "libmozok/public_types.hpp"

using namespace mozok;
using namespace mozok::app;
using namespace std;


namespace {

using CommandFunc = Result (*)(App&, const Str&, const StrVec&);
std::unordered_map<Str, CommandFunc> commandMap;

using OptionFunc = Result (*)(AppOptions&, int, char**, int&);
std::unordered_map<Str, OptionFunc> optionMap;


// ============================ Print Functions ============================ //

void print_CallAppHelp() {
    cout << "Expecting:\n > mozok <script_file_path> [<options>]" << endl;
    cout << "Call `mozok -h` to get more information on how use this tool.\n";
    cout << "Call `mozok -h <option>` to get more information about";
    cout << " a specific option." << endl;
}

void print_AppHelp() {
    print_CallAppHelp();
    cout << "Avaiable options:" << endl;
    for(const auto& h : HELP_MAP)
        if(h.second.flags == APP_OPTION)
            cout << "  " << h.second.name << " - " << h.second.brief << endl;
}

void print_UnknownOption(const Str& option) {
    cout << "ERROR: Unknown option `" << option << "`." << endl;
}

void print_OptionHelp(const Str& option) {
    const auto it = HELP_MAP.find(option);
    if(it == HELP_MAP.end()) {
        print_UnknownOption(option);
        return;
    }
    const auto& h = it->second;
    if(h.flags != APP_OPTION) {
        print_UnknownOption(option);
        return;
    }
    cout << "Format: " << h.format << endl;
    cout << "Description: " << h.desc << endl;
    if(h.args.size() > 0) {
        cout << "Argument(s):" << endl;
        for(const auto& a : h.args)
            cout << " * " << a << endl;
    }
}

void print_Error(const Str& msg) {
    cout << "ERROR: " << msg << endl;
}

void print_ErrorResult(const Result& r) {
    cout << "ERROR: Full error message:\n" << r.getDescription() << endl;
}

void print_UnknownCommand(const Str& command) {
    cout << "ERROR: Unknown command `" << command << "`." << endl;
}

void print_CallHelpMsg() {
    cout << "Call `help` to get the general help information." << endl;
    cout << "Call `help <command>` for help on a specific command" << endl;
    cout << "Call `exit` for help on a specific command." << endl;
}

void print_GeneralHelp() {
    print_CallHelpMsg();
    cout << endl << "Other available commands:" << endl;
    for(const auto& h : HELP_MAP) {
        if(commandMap.find(h.first) == commandMap.end())
            continue;
        cout << " > " << h.first; 
        cout << " - " << h.second.brief;
        cout << endl;
    }
    cout << endl;
}

void print_CommandHelp(const Str& c) {
    bool unknown = false;
    const auto& it = HELP_MAP.find(c);
    unknown |= it == HELP_MAP.end();
    unknown |= commandMap.find(c) == commandMap.end();
    if(unknown) {
        print_UnknownCommand(c);
        print_CallHelpMsg();
    } else {
        cout << "Format:\n > " << it->second.format << endl;
        cout << it->second.desc << endl;
        if(it->second.args.size() > 0) {
            cout << "Arguments:" << endl;
            for(const auto& a : it->second.args)
                cout << " * " << a << endl;
        }
        cout << endl;
    }
}

[[nodiscard]] Result print_BadOptionFormat(const Str& option) {
    return Result::Error("Bad `" + option + "` option format."
                         " Call `help " + option + "`.");
}

[[nodiscard]] Result print_BadCommandFormat(const Str& command) {
    print_CallHelpMsg();
    return Result::Error("Bad `" + command + "` command format."
                         " Call `help " + command + "`.");
}


// =========================== Options Functions =========================== //

Result o_pauseOnError(AppOptions& appOptions, int, char**, int&) {
    appOptions.pauseOnError = true;
    return Result::OK();
}

Result o_printOnOk(AppOptions& appOptions, int argc, char** argv, int& p) {
    if(++p >= argc)
        return print_BadOptionFormat(argv[p-1]);
    appOptions.printOnOk = argv[p];
    return Result::OK();
}

Result o_setServerName(AppOptions& appOptions, int argc, char** argv, int& p) {
    if(++p >= argc)
        return print_BadOptionFormat(argv[p-1]);
    appOptions.serverName = argv[p];
    return Result::OK();
}

Result o_noInit(AppOptions& appOptions, int, char**, int&) {
    appOptions.applyInitAction = false;
    return Result::OK();
}


// =========================== Command Functions =========================== //

Result c_help(App&, const Str&, const StrVec& tokens) {
    if(tokens.size() == 1) {
        print_GeneralHelp();
    } else if(tokens.size() == 2) {
        const Str& c = tokens[1];
        if(c == "help") {
            print_CallHelpMsg();
            return Result::OK();
        }
        print_CommandHelp(c);
    } else
        return print_BadCommandFormat("help");
    return Result::OK();
}

Result c_world(App& app, const Str&, const StrVec& tokens) {
    if(tokens.size() != 2)
        return print_BadCommandFormat(tokens[0]);
    return app.newWorld(tokens[1]);
}

Result c_info(App& app, const Str&, const StrVec&) {
    Str info = app.getInfo();
    cout << "INFO:\n" << endl;
    cout << info << endl;
    return Result::OK();
}

Result c_block_cmd(App& app, const Str& line, const StrVec&) {
    Str cmd = line;
    if(line == "exit")
        cmd += " Normal exit";
    return QSFParser::parseAndApplyCmd(cmd, &app);
}

}

// =============================== Callback ================================ //

class Callback : public AppCallback {
public:
    Callback() : AppCallback() 
    { /*empty*/ }

    bool onPause(App* app) noexcept override {
        while(true) {
            cout << ">> ";
            Str line;
            getline(cin, line);

            // Split the command line into tokens.
            stringstream ss(line);
            std::istream_iterator<Str> b(ss);
            std::istream_iterator<Str> e;
            StrVec tokens(b,e);
            if(tokens.size() == 0) {
                print_CallHelpMsg();
                continue;
            }

            const Str& command = tokens[0];

            if(command == C_CONTINUE)
                break;
            
            if(commandMap.find(command) != commandMap.end()) {
                Result r = commandMap[command](*app, line, tokens);
                if(r.isError())
                    print_ErrorResult(r);
            } else {
                print_UnknownCommand(command);
                print_CallHelpMsg();
                continue;
            }
            
            if(command == C_EXIT)
                return false;
        }
        return true;
    }

    void onError() noexcept override {
    }

} callback;

// ================================= Main ================================== //

const int ERROR_CODE = 1;

int main(int argc, char **argv) {
    if(argc < 2) {
        print_CallAppHelp();
        return 0;
    }

    AppOptions appOptions;
    const Str scriptFileName = argv[1];

    // Show app help.
    if(scriptFileName == O_HELP) {
        if(argc == 2)
            print_AppHelp();
        else if(argc == 3)
            print_OptionHelp(argv[2]);
        else
            print_CallAppHelp();
        return 0;
    }

    // Read the input script file.
    ifstream in(scriptFileName);
    if(in.is_open() == false) {
        print_Error("Can't open the file `" + scriptFileName + "`.");
        return 0;
    }
    stringstream in_buffer;
    in_buffer << in.rdbuf();
    const Str scriptFile(in_buffer.str());
    in.close();

    // Setting up the map that maps command name into a command function.
    commandMap["help"] = &c_help;
    //commandMap[C_INFO] = ;
    commandMap[C_EXIT] = &c_block_cmd;
    commandMap[C_CONTINUE] = nullptr;
    commandMap[C_WORLD] = &c_world;
    commandMap[C_INFO] = &c_info;
    commandMap[C_PRINT] = &c_block_cmd;
    commandMap[C_EXPECT] = &c_block_cmd;
    commandMap[C_APPLY] = &c_block_cmd;

    // Setting up the map that maps options name into an option function.
    optionMap[O_PAUSE_ON_ERR] = &o_pauseOnError;
    optionMap[O_NO_INIT] = &o_noInit;
    optionMap[O_PRINT_ON_OK] = &o_printOnOk;
    optionMap[O_SERVER_NAME] = &o_setServerName;

    // Turning on the options.
    for(int p = 2; p < argc; ++p) {
        auto it = optionMap.find(argv[p]);
        if(it == optionMap.end()) {
            print_UnknownOption(argv[p]);
            print_CallAppHelp();
            return ERROR_CODE;
        }
        it->second(appOptions, argc, argv, p);
    }

    // Create the application.
    Result status;
    unique_ptr<App> app(App::create(appOptions, status));
    if(status.isError() || app==nullptr || app->getCurrentStatus().isError()) {
        print_Error("Oops! Can't create the `App` instance.");
        print_ErrorResult(status);
        return ERROR_CODE;
    }

    // Parse the input QSF into the application.
    status <<= QSFParser::parse(scriptFileName, scriptFile, app.get());
    if(status.isError()) {
        cout << status.getDescription() << endl;
        return ERROR_CODE;
    }

    status <<= app->simulate(&callback);
    if(status.isOk()) { 
        if(appOptions.printOnOk.length() > 0)
            cout << appOptions.printOnOk << endl;
    }

    return 0;
}

