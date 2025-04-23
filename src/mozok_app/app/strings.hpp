// Copyright 2025 Pavlo Savchuk. Subject to the MIT license.

// String constants commonly used by the mozok app.

#pragma once

#include <libmozok/public_types.hpp>
#include <map>

namespace mozok {
namespace app {

/// @brief Standard error message printed by the debugger.
const Str ERROR_MSG = "MOZOK_ERROR";

enum HelpFlag {
    APP_OPTION = 0,
    SCRIPT_COMMAND = 1,
    TERMINAL_COMMAND = 2,
    GENERAL_COMMAND = SCRIPT_COMMAND | TERMINAL_COMMAND
};

struct HelpInfo {
    HelpFlag flags;
    Str name;
    Str format;
    Str brief;
    Str desc;
    StrVec args;
};

using HelpMap = std::map<Str, HelpInfo>;

#define MOZOK_HELP(f, c) \
    { c, {f, c, c##_FORMAT, c##_BRIEF, c##_DESC, c##_ARGS } }


// ------------------------------------------------------------------------- //

/// @defgroup Application
/// @{

const Str O_HELP = "-h";
const Str O_HELP_FORMAT = "-h";
const Str O_HELP_BRIEF = "Print the general help information.";
const Str O_HELP_DESC = "Print (on standard output) the general help"
    " information on how to use the `mozok` tool.";
const StrVec O_HELP_ARGS = {};

const Str O_PAUSE_ON_ERR = "-p";
const Str O_PAUSE_ON_ERR_FORMAT = "-p";
const Str O_PAUSE_ON_ERR_BRIEF = "Pause the app on error.";
const Str O_PAUSE_ON_ERR_DESC = "With this option set, on error, instead"
    " of closing after the error message, the app will pause the simulation"
    " and show the debug console.";
const StrVec O_PAUSE_ON_ERR_ARGS = {};

const Str O_PRINT_ON_OK = "-P";
const Str O_PRINT_ON_OK_FORMAT = "-P <message>";
const Str O_PRINT_ON_OK_BRIEF = "Print a <message> on success.";
const Str O_PRINT_ON_OK_DESC = O_PRINT_ON_OK_BRIEF;
const StrVec O_PRINT_ON_OK_ARGS = 
    { "`<message>` - This message will be printed (on std output)"
      " if no errors wasn't occured during the simulation." };

const Str O_SERVER_NAME = "-s";
const Str O_SERVER_NAME_FORMAT = "-s <server_name>";
const Str O_SERVER_NAME_BRIEF = "Sets the quest server name.";
const Str O_SERVER_NAME_DESC = O_SERVER_NAME_BRIEF;
const StrVec O_SERVER_NAME_ARGS = 
    { "`<server_name>` - New server name (default `mozok_app`)." };

const Str O_NO_INIT = "-n";
const Str O_NO_INIT_FORMAT = "-n";
const Str O_NO_INIT_BRIEF = "(No-init) Do not call the `init` actions.";
const Str O_NO_INIT_DESC = "If set, the app will not call the init actions.";
const StrVec O_NO_INIT_ARGS = {};

const Str O_VERBOSE = "-V";
const Str O_VERBOSE_FORMAT = "-V";
const Str O_VERBOSE_BRIEF = "Verbose output.";
const Str O_VERBOSE_DESC = "Turn ON the verbose output mode.";
const StrVec O_VERBOSE_ARGS = {};

const Str O_EXPORT_GRAPH = "-g";
const Str O_EXPORT_GRAPH_FORMAT = "-g <filename.gv>";
const Str O_EXPORT_GRAPH_BRIEF = "Exports the simulation graph into a .gv file.";
const Str O_EXPORT_GRAPH_DESC = O_EXPORT_GRAPH_BRIEF;
const StrVec O_EXPORT_GRAPH_ARGS = 
    { "`<filename.gv>` - Exports into this file in .gv (Graphviz DOT) format." };

const Str O_EXPORT_FLAGS = "-f";
const Str O_EXPORT_FLAGS_FORMAT = "-f <flags>";
const Str O_EXPORT_FLAGS_BRIEF = "Graph export visibility flags (default: mb).";
const Str O_EXPORT_FLAGS_DESC = 
        O_EXPORT_FLAGS_BRIEF + " You can set multiple flags, for example: mpPd.";
const StrVec O_EXPORT_FLAGS_ARGS = 
    { "p - Push action blocks."
    , "m - Meta blocks (PRINT, PAUSE, EXIT)."
    , "e - Event blocks."
    , "b - Command blocks (ACT_IF, ALWAYS etc.)"
    , "x - Expect blocks."
    , "P - Plan Accepted/Changed blocks."
    , "E - Action error blocks."
    , "d - Include details."
    };

const Str O_MAX_WAIT_TIME = "-w";
const Str O_MAX_WAIT_TIME_FORMAT = "-w <max_wait_time_ms>";
const Str O_MAX_WAIT_TIME_BRIEF = "Sets the maximum wait time in ms (default: 5000).";
const Str O_MAX_WAIT_TIME_DESC = O_SERVER_NAME_BRIEF 
        + " An error will occur if no events are received"
        + " for a duration longer than this.";
const StrVec O_MAX_WAIT_TIME_ARGS = 
    { "`<max_wait_time_ms>` - Maximum wait time in milliseconds (positive integer)." };

/// @}

// ------------------------------------------------------------------------- //

/// @defgroup Commands
/// @{

const Str C_EXIT = "exit";
const Str C_EXIT_FORMAT = "exit [<text>]";
const Str C_EXIT_BRIEF = "Immedianty closes the mozok app.";
const Str C_EXIT_DESC = "Prints a message `exit [<text>]`"
    " and immediately closes the mozok app.";
const StrVec C_EXIT_ARGS = 
    { "`<text>` - If set, before exiting, it will print"
        " the `exit <text>` message."
    };

const Str C_PAUSE = "pause";
const Str C_PAUSE_FORMAT = "pause [<breakpoint_name>]";
const Str C_PAUSE_BRIEF = "Pauses the debugger and opens a debug terminal.";
const Str C_PAUSE_DESC = C_PAUSE_BRIEF;
const StrVec C_PAUSE_ARGS =
    { "`<breakpoint_name>` - If set, forces to print"
        " `STOPPED AT <breakpoint>` into a debug terminal."
        " Useful when script has multiple `pause` commands." 
    };

const Str C_CONTINUE = "continue"; 
const Str C_CONTINUE_FORMAT = "continue";
const Str C_CONTINUE_BRIEF = "Continue the simulation process.";
const Str C_CONTINUE_DESC = C_CONTINUE_BRIEF;
const StrVec C_CONTINUE_ARGS = {};

const Str C_INFO = "info"; 
const Str C_INFO_FORMAT = "info";
const Str C_INFO_BRIEF = "Print a general info about the current state.";
const Str C_INFO_DESC = C_INFO_BRIEF;
const StrVec C_INFO_ARGS = {};

const Str C_PRINT = "print";
const Str C_PRINT_FORMAT = "print [<text>]";
const Str C_PRINT_BRIEF = "Prints a text message `print <text>`.";
const Str C_PRINT_DESC = C_PRINT_BRIEF;
const StrVec C_PRINT_ARGS = {};

const Str C_WORLD = "world";
const Str C_WORLD_FORMAT = "world <name>";
const Str C_WORLD_BRIEF = "Creates a new quest world with a unique name.";
const Str C_WORLD_DESC = C_WORLD_BRIEF;
const StrVec C_WORLD_ARGS = 
    { "`<name>` - The name of the world that must be created."
        " Make sure each quest world has a unique name. If a world with"
        " this name aldeary exists it throws an error message and"
        " closes the app"
    };

const Str C_LOAD = "load";
const Str C_LOAD_FORMAT = "load <[world]> <file_path>";
const Str C_LOAD_BRIEF = "Loads a quest project into a quest world.";
const Str C_LOAD_DESC = C_LOAD_BRIEF;
const StrVec C_LOAD_ARGS = 
    { "`<[world]>` - The name of the world !in brackets!, into which we load a project"
    , "`<file_path>` - The path to the `.quest` project file"
    };

const Str C_EXPECT = "expect";
const Str C_EXPECT_FORMAT = "expect UNREACHABLE <[world]> <quest>";
const Str C_EXPECT_BRIEF = "Expect quest to fail.";
const Str C_EXPECT_DESC = "By default any active quest is expected to be `DONE`."
        " After this command, the <quest> is expected to be `UNREACHABLE`.";
const StrVec C_EXPECT_ARGS =
    { "`<[world]>` - The name of the world !in brackets!, into which we load a project"
    , "`<quest>` - The name of the quest expected to be UNREACHABLE"
    };

const Str C_PUSH = "push";
const Str C_PUSH_FORMAT = "push <[world]> <action>([<arguments>])";
const Str C_PUSH_BRIEF = "Push an action into the worker thread queue.";
const Str C_PUSH_DESC = C_PUSH_BRIEF;
const StrVec C_PUSH_ARGS =
    { "`<[world]>` - The name of the world !in brackets!, into which we load a project"
    , "`<action>` - Action name"
    , "`[<arguments>] - Action arguments`"
    };

/// @}

// ------------------------------------------------------------------------- //

/// @brief Maps command name into command's help information struct.
const HelpMap HELP_MAP = {
    // options
      MOZOK_HELP(APP_OPTION, O_HELP)
    , MOZOK_HELP(APP_OPTION, O_PAUSE_ON_ERR)
    , MOZOK_HELP(APP_OPTION, O_PRINT_ON_OK)
    , MOZOK_HELP(APP_OPTION, O_SERVER_NAME)
    , MOZOK_HELP(APP_OPTION, O_NO_INIT)
    , MOZOK_HELP(APP_OPTION, O_VERBOSE)
    , MOZOK_HELP(APP_OPTION, O_EXPORT_GRAPH)
    , MOZOK_HELP(APP_OPTION, O_EXPORT_FLAGS)
    , MOZOK_HELP(APP_OPTION, O_MAX_WAIT_TIME)
    
    // commands
    , MOZOK_HELP(GENERAL_COMMAND, C_EXIT)
    , MOZOK_HELP(SCRIPT_COMMAND, C_PAUSE)
    , MOZOK_HELP(TERMINAL_COMMAND, C_CONTINUE)
    , MOZOK_HELP(GENERAL_COMMAND, C_PRINT)
    , MOZOK_HELP(TERMINAL_COMMAND, C_INFO)
    , MOZOK_HELP(TERMINAL_COMMAND, C_WORLD)
    , MOZOK_HELP(TERMINAL_COMMAND, C_LOAD)
    , MOZOK_HELP(GENERAL_COMMAND, C_EXPECT)
    , MOZOK_HELP(GENERAL_COMMAND, C_PUSH)
};


}
}
