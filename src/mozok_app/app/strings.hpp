// Copyright 2025 Pavlo Savchuk. Subject to the MIT license.

// String constants, commonly used by the mozok app.

#pragma once

#include <libmozok/public_types.hpp>
#include <map>

namespace mozok {
namespace app {

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

/// @}


// ------------------------------------------------------------------------- //

/// @defgroup Commands
/// @{



const Str C_EXIT = "exit";
const Str C_EXIT_FORMAT = "exit [<text>]";
const Str C_EXIT_BRIEF = "Immedianty closes the mozok app.";
const Str C_EXIT_DESC = "Prints a message `exit [<text>]`"
    " and immedianty closes the mozok app.";
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


/*const Str C_STD_WORLD = "std_world";
const Str C_STD_WORLD_FORMAT = "std_world <world_name>";
const Str C_STD_WORLD_BRIEF = "Sets the standard world name.";
const Str C_STD_WORLD_DESC = 
    "Sets a previously created `<world_name>` as the standard world name."
    " If standard world is set, then instead of `world.obj` format you can use"
    " just `obj`. You can only call this command once per script.";
const StrVec C_STD_WORLD_ARGS = 
    { "`<world_name>` - The name of a quest world that will"
        " be used as the standard quest world."
    };*/


const Str C_LOAD = "load";
const Str C_LOAD_FORMAT = "load <[world]> <file_path>";
const Str C_LOAD_BRIEF = "Loads a quest project into a quest world.";
const Str C_LOAD_DESC = C_LOAD_BRIEF;
const StrVec C_LOAD_ARGS = 
    { "`<[world]>` - The name of the world !in brackets!, into which we load a project"
    , "`<file_path>` - The path to the `.quest` project file"
    };


const Str C_EXPECT = "expect";
const Str C_EXPECT_FORMAT = "expect <what> <args>";
const Str C_EXPECT_BRIEF = "...";
const Str C_EXPECT_DESC = "...";
const StrVec C_EXPECT_ARGS =
    { "`<what>` - "
    , " ... "
    };

const Str C_APPLY = "apply";
const Str C_APPLY_FORMAT = "apply <[world]> <action>";
const Str C_APPLY_BRIEF = "...";
const Str C_APPLY_DESC = "...";
const StrVec C_APPLY_ARGS =
    { " ... "
    , " ... "
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
    
    // commands
    , MOZOK_HELP(GENERAL_COMMAND, C_EXIT)
    , MOZOK_HELP(SCRIPT_COMMAND, C_PAUSE)
    , MOZOK_HELP(TERMINAL_COMMAND, C_CONTINUE)
    , MOZOK_HELP(GENERAL_COMMAND, C_PRINT)
    , MOZOK_HELP(TERMINAL_COMMAND, C_INFO)
    , MOZOK_HELP(TERMINAL_COMMAND, C_WORLD)
    //, MOZOK_HELP(GENERAL_COMMAND, C_STD_WORLD)
    , MOZOK_HELP(TERMINAL_COMMAND, C_LOAD)
    , MOZOK_HELP(GENERAL_COMMAND, C_EXPECT)
    , MOZOK_HELP(GENERAL_COMMAND, C_APPLY)
};


}
}
