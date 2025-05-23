// Copyright 2024-2025 Pavlo Savchuk. Subject to the MIT license.

#include <libmozok/public_types.hpp>
#include <libmozok/quest_manager.hpp>
#include <libmozok/project.hpp>
#include <libmozok/world.hpp>
#include <libmozok/parser.hpp>
#include <libmozok/error_utils.hpp>

namespace mozok {

namespace {
    const int PROJECT_FORMAT_VMAJOR = 1;
    const int PROJECT_FORMAT_VMINOR = 0;

    const char* KEYWORD_VERSION = "version";
    const char* KEYWORD_PROJECT = "project";
    const char* KEYWORD_TYPE = "type";
    const char* KEYWORD_OBJECT = "object";
    const char* KEYWORD_OBJECTS = "objects";
    const char* KEYWORD_REL = "rel";
    const char* KEYWORD_RLIST = "rlist";
    const char* KEYWORD_AGROUP = "agroup";
    const char* KEYWORD_ACTION = "action";
    const char* KEYWORD_PRE = "pre";
    const char* KEYWORD_REM = "rem";
    const char* KEYWORD_ADD = "add";
    const char* KEYWORD_QUEST = "quest";
    const char* KEYWORD_MAIN_QUEST = "main_quest";
    const char* KEYWORD_PRECONDITIONS = "preconditions";
    const char* KEYWORD_GOAL = "goal";
    const char* KEYWORD_ACTIONS = "actions";
    const char* KEYWORD_SUBQUESTS = "subquests";
    const char* KEYWORD_STATUS = "status";
    const char* KEYWORD_PARENT = "PARENT";
    const char* KEYWORD_INACTIVE = "INACTIVE";
    const char* KEYWORD_ACTIVE = "ACTIVE";
    const char* KEYWORD_DONE = "DONE";
    const char* KEYWORD_UNREACHABLE = "UNREACHABLE";
    const char* KEYWORD_NA = "N/A";
    const char* KEYWORD_OPTIONS = "options";
    const char* KEYWORD_SEARCH_LIMIT = "searchLimit";
    const char* KEYWORD_SPACE_LIMIT = "spaceLimit";
    const char* KEYWORD_OMEGA = "omega";
    const char* KEYWORD_HEURISTIC = "heuristic";
    const char* KEYWORD_SIMPLE = "SIMPLE";
    const char* KEYWORD_HSP = "HSP";
    const char* KEYWORD_USE_ATREE = "use_atree";
    const char* KEYWORD_STRATEGY = "strategy";
    const char* KEYWORD_ASTAR = "ASTAR";
    const char* KEYWORD_DFS = "DFS";
}


/// @brief Recursive descent parser for .quest files.
class QuestProjectParser : public RecursiveDescentParser {
    World* _world;

    /// @brief Here the major version of the .quest file will be written.
    int _majorVersion;

    /// @brief Here the minor version of the .quest file will be written.
    int _minorVersion;

    /// @brief This field will store the project name read from the .quest file.
    Str _projectName;

public:
    QuestProjectParser(
            World* world, 
            const Str& file, 
            const Str& projectSrc) noexcept : 
        RecursiveDescentParser(file, projectSrc, true),
        _world(world),
        _majorVersion(-1),
        _minorVersion(-1)
    { /* empty */ }

    /// @brief Parses a `version X Y` command.
    /// @return Returns 'Result::OK()' if the reading operation was successful.
    ///         Otherwise, return a detailed error message.
    Result version() noexcept {
        Result res = space(0);
        if((res = keyword(KEYWORD_VERSION)).isError())
            return res;
        if((res = space(1)).isError())
            return res;
        if((res = pos_int(_majorVersion)).isError())
            return res;
        if((res = space(1)).isError())
            return res;
        if((res = pos_int(_minorVersion)).isError())
            return res;
        empty_lines();
        if(_majorVersion != PROJECT_FORMAT_VMAJOR || _minorVersion != PROJECT_FORMAT_VMINOR)
            return errorParserUnsupportedVersion(
                    _file, _line, _col, 
                    PROJECT_FORMAT_VMAJOR, PROJECT_FORMAT_VMINOR,
                    _majorVersion, _minorVersion);
        return Result::OK();
    }

    /// @brief Parses a 'project project_name' command.
    /// @return Returns 'Result::OK()' if the reading operation was successful.
    ///         Otherwise, return a detailed error message.
    Result project() noexcept {
        Result res = space(0);
        if((res = keyword(KEYWORD_PROJECT)).isError())
            return res;
        if((res = space(1)).isError())
            return res;
        res = name(_projectName);
        return res;
    }

    
    /// @brief Parses a type name. Type names must begin with an uppercase letter.
    ///        Also, can check if the type name is defined (this checking is not
    ///        necessary, but it leads to more straight forward error message).
    /// @param out The parsed name will be written into this variable.
    /// @return Returns 'Result::OK()' if the reading operation was successful.
    ///         Otherwise, return a detailed error message.
    Result type(Str& out, bool checkIfDefined = true) noexcept {
        Result res = name(out, UPPER);
        if(checkIfDefined && _world->hasType(out) == false) {
            res <<= errorUndefinedType(_world->getServerWorldName(), out);
            res <<= errorParserWorldError(
                    _file, _line, _col, _world->getServerWorldName());
        }
        return res;
    }

    /// @brief Parses a list of comma-separated defined types.
    /// @param out The parsed list will be added into this variable.
    /// @param allowEmpty Sets if empty list is allowed.
    /// @return Returns 'Result::OK()' if the reading operation was successful.
    ///         Otherwise, return a detailed error message.
    Result type_list(StrVec& out, bool allowEmpty = false) noexcept {
        Result res;
        Str tmpTypeName;
        int count = 0;
        do {
            res <<= space(0);
            res <<= type(tmpTypeName);
            res <<= space(0);
            if(res.isError())
                return (allowEmpty && count == 0 ? Result::OK() : res);
            out.push_back(tmpTypeName);
            ++count;
        }
        while(comma().isOk());
        return res;
    }

    /// @brief Parses a list of comma-separated objects and/or variables.
    /// @param out The parsed list will be added into this variable.
    /// @return Returns 'Result::OK()' if the reading operation was successful.
    ///         Otherwise, return a detailed error message.
    Result object_list(StrVec& out) noexcept {
        Result res;
        Str tmpObjName;
        bool firstArgument = true;
        do {
            res <<= space(0);
            res <<= name(tmpObjName, LOWER);
            res <<= space(0);
            if(res.isError()) {
                if(firstArgument)
                    // no objects where given 
                    // (probably a zero-arity relation)
                    return Result::OK();
                return res;
            }
            out.push_back(tmpObjName);
            firstArgument = false;
        }
        while(comma().isOk());
        return res;
    }

    /// @brief Parses a type definition.
    /// @return Returns 'Result::OK()' if the reading operation was successful.
    ///         Otherwise, return a detailed error message.
    Result type_definition() noexcept {
        Str newTypeName;
        StrVec supertypeNames;
        Result res;

        res <<= type(newTypeName, false);
        if(colon_with_spaces().isOk())
            res <<= type_list(supertypeNames);
        const int commandLine = _line;
        res <<= next_line();
        if(res.isError())
            return res;
        
        res <<= _world->addType(newTypeName, supertypeNames);
        if(res.isError())
            res <<= errorParserWorldError(
                    _file, commandLine, _col, _world->getServerWorldName());
        return res;
    }

    /// @brief Parses an object definition.
    /// @return Returns 'Result::OK()' if the reading operation was successful.
    ///         Otherwise, return a detailed error message.
    Result object_definition() noexcept {
        Str objName;
        StrVec objType;
        Result res;

        res <<= name(objName, LOWER);
        res <<= colon_with_spaces();
        res <<= type_list(objType);

        const int commandLine = _line;
        res <<= next_line();
        if(res.isError())
            return res;

        res <<= _world->addObject(objName, objType);
        if(res.isError())
            res <<= errorParserWorldError(
                    _file, commandLine, _col, _world->getServerWorldName());
        return res;
    }
    
    /// @brief Parses a relation definition.
    /// @return Returns 'Result::OK()' if the reading operation was successful.
    ///         Otherwise, return a detailed error message.
    Result relation_definition() noexcept {
        Str relName;
        StrVec argTypes;
        Result res;

        res <<= name(relName, UPPER);
        res <<= space(0);
        res <<= par_open();
        res <<= space(0);
        res <<= type_list(argTypes, true);
        res <<= space(0);
        res <<= par_close();
        res <<= space(0);

        const int commandLine = _line;
        res <<= next_line();
        if(res.isError())
            return res;

        res <<= _world->addRelation(relName, argTypes);
        if(res.isError())
            res <<= errorParserWorldError(
                    _file, commandLine, _col, _world->getServerWorldName());
        return res;
    }

    /// @brief Parses a vertical list of rlist/action arguments.
    /// @param out The parsed list will be added into this variable.
    ///         Format: [ ["name", "TypeName", "TypeName", ...], ... ].
    /// @return Returns 'Result::OK()' if the reading operation was successful.
    ///         Otherwise, return a detailed error message.
    Result argument_list(Vector<StrVec> &out) noexcept {
        Result res;

        while(true) {
            Str argName;
            StrVec argNameAndTypes;
            res <<= space(1);
            res <<= name(argName, LOWER);
            res <<= colon_with_spaces();

            if(res.isError()) {
                // No more arguments found.
                _pos -= _col;
                _col = 0;
                break;
            }

            argNameAndTypes.push_back(argName);

            res <<= type_list(argNameAndTypes);
            res <<= next_line();
            res <<= empty_lines();

            if(res.isError())
                return res;

            out.push_back(argNameAndTypes);
        }
        return Result::OK();
    }

    /// @brief Parses a vertical list of relations and rlists.
    /// @param out The parsed list will be added into this variable.
    ///         Format: [ ["Name", "objectName", "objectName", ...], ... ].
    /// @return Returns 'Result::OK()' if the reading operation was successful.
    ///         Otherwise, return a detailed error message.
    Result change_list(Vector<StrVec> &out) noexcept {
        Result res;

        // (Checking for the next scenario):
        // <some_text>[cursor] # Comment or an empty space
        //      ChangeListItem1(...)
        //      ChangeListItem2(...)
        //      ...
        int line = _line;
        int col = _col;
        int pos = _pos;
        res <<= empty_lines();
        if(_line == line) {
            // If the current line is the same as the initial line,
            // it means there was no empty line after the cursor.
            // In this case, return the previous state.
            _col = col;
            _pos = pos;
        }

        while(true) {
            Str commandName;
            StrVec commandWithArguments;
            res <<= space(1);
            res <<= name(commandName, UPPER);
            res <<= space(0);

            if(res.isError()) {
                // No more commands found.
                _pos -= _col;
                _col = 0;
                break;
            }

            commandWithArguments.push_back(commandName);

            res <<= space(0);
            res <<= par_open();
            res <<= space(0);
            res <<= object_list(commandWithArguments);
            res <<= space(0);
            res <<= par_close();
            res <<= space(0);
            res <<= next_line();
            res <<= empty_lines();

            if(res.isError())
                return res;

            out.push_back(commandWithArguments);
        }
        return Result::OK();
    }

    /// @brief Parses a relation list definition.
    /// @return Returns 'Result::OK()' if the reading operation was successful.
    ///         Otherwise, return a detailed error message.
    Result rlist_definition() noexcept {
        Str rlistName;
        Result res;

        res <<= name(rlistName, UPPER);
        res <<= colon_with_spaces();
        if(res.isError())
            return res;

        const int commandLine = _line;
        res <<= next_line();
        if(res.isError())
            return res;

        Vector<StrVec> arguments;
        res <<= empty_lines();
        res <<= argument_list(arguments);
        if(res.isError())
            return res;
        
        Vector<StrVec> changeList;
        res <<= empty_lines();
        res <<= change_list(changeList);
        if(res.isError())
            return res;

        res <<= _world->addRelationList(rlistName, arguments, changeList);
        if(res.isError())
            res <<= errorParserWorldError(
                    _file, commandLine, _col, _world->getServerWorldName());
        return res;
    }

    /// @brief Parses `N/A` keyword.
    /// @param out Writes `true` into `out` if `N/A` keyword was present.
    ///            Writes `false` if `N/A` keyword was not present.
    /// @return Always returns 'Result::OK()'.
    Result read_na(bool& out) {
        int tmpPos = _pos;
        int tmpCol = _col;
        if(keyword(KEYWORD_NA).isOk()) {
            out = true;
        } else {
            _pos = tmpPos;
            _col = tmpCol;
            out = false;
        }
        return Result::OK();
    }
    
    /// @brief Parses an action group definition.
    /// @return Returns 'Result::OK()' if the reading operation was successful.
    ///         Otherwise, return a detailed error message.
    Result action_group_definition() noexcept {
        Str groupName;
        Result res;
        res <<= name(groupName, Case::LOWER);
        res <<= space(0);
        res <<= next_line();
        if(res.isError())
            return res;
        res <<= _world->addActionGroup(groupName);
        if(res.isError())
            res <<= errorParserError(_file, _line, _col, 
                                     "Action group error. See prev. error.");
        return res;
    }

    /// @brief Parses action groups.
    Result action_groups(StrVec& out) noexcept {
        Result res;
        out.clear();
        if(curly_bracket_open().isError())
            return Result::OK();

        Str groupName;
        do {
            res <<= space(0);
            res <<= name(groupName, Case::LOWER);
            if(res.isError())
                return res <<= errorParserError(
                        _file, _line, _col, 
                        "Expecting action group name. See prev. error");
            if(_world->hasActionGroup(groupName) == false)
                return res <<= errorParserError(
                        _file, _line, _col, 
                        "Undefined action group `" + groupName + "`.");
            out.push_back(groupName);
            res <<= space(0);
        } while(comma().isOk());

        res <<= space(0);
        res <<= curly_bracket_close();
        return res;
    }

    /// @brief Parses an action definition.
    /// @return Returns 'Result::OK()' if the reading operation was successful.
    ///         Otherwise, return a detailed error message.
    Result action_definition() noexcept {
        Str actionName;
        StrVec actionGroups;
        bool isNotApplicable = false;
        Result res;

        res <<= read_na(isNotApplicable);
        res <<= space(0);

        res <<= name(actionName, UPPER);
        res <<= space(0);
        res <<= action_groups(actionGroups);
        res <<= colon_with_spaces();
        if(res.isError())
            return res;

        const int commandLine = _line;
        res <<= next_line();
        if(res.isError())
            return res;

        Vector<StrVec> arguments;
        res <<= empty_lines();
        res <<= argument_list(arguments);
        if(res.isError())
            return res;
        
        // Read the status change commands.
        StrVec quests;
        Vector<QuestStatus> statusList;
        Vector<int> goals;
        UnorderedMap<Str, Str> parentQuests;
        UnorderedMap<Str, int> parentGoals;
        res <<= empty_lines();
        res <<= space(0);
        while(keyword(KEYWORD_STATUS).isOk()) {
            Str questName, status;
            int goal = 0;
            res <<= space(0);
            res <<= name(questName, UPPER);
            res <<= space(0);
            int statusCol = _col;
            int statusLine = _line;
            res <<= name(status, UPPER);
            if((status == KEYWORD_ACTIVE) || (status == KEYWORD_DONE)) {
                res <<= space(0);
                res <<= pos_int(goal);
            }
            res <<= space(0);
            if(next_line().isError()) {
                // Maybe this is a subquest?
                int parentCol = _col;
                int parentLine = _line;
                if(keyword(KEYWORD_PARENT).isOk()) {
                    // It is a subquest.
                    res <<= space(0);
                    Str parentQuestName;
                    int parentQuestGoal;
                    res <<= name(parentQuestName, UPPER);
                    res <<= space(0);
                    res <<= pos_int(parentQuestGoal);
                    res <<= space(0);
                    res <<= empty_lines();
                    parentQuests[questName] = parentQuestName;
                    parentGoals[questName] = parentQuestGoal;
                } else
                    return errorParserError(
                            _file, parentLine, parentCol, 
                            "Expecting a new line or `PARENT`");
            }
            res <<= empty_lines();
            quests.push_back(questName);
            goals.push_back(goal);
            if(status == KEYWORD_INACTIVE)
                statusList.push_back(MOZOK_QUEST_STATUS_INACTIVE);
            else if(status == KEYWORD_ACTIVE)
                statusList.push_back(MOZOK_QUEST_STATUS_UNKNOWN);
            else if(status == KEYWORD_DONE)
                statusList.push_back(MOZOK_QUEST_STATUS_DONE);
            else if(status == KEYWORD_UNREACHABLE)
                statusList.push_back(MOZOK_QUEST_STATUS_UNREACHABLE);
            else
                return errorActionInvalidStatus(_file, statusLine, statusCol);
            res <<= space(0);
            if(res.isError())
                return res;
        }
        _pos -= _col;
        _col = 0;

        Vector<StrVec> preList;
        Vector<StrVec> remList;
        Vector<StrVec> addList;

        const char* keywords[] = {KEYWORD_PRE, KEYWORD_REM, KEYWORD_ADD};
        Vector<StrVec>* lists[] = {&preList, &remList, &addList};

        for(int i=0; i<3; ++i) {
            res <<= empty_lines();
            res <<= space(0);
            res <<= keyword(keywords[i]);
            if(res.isError()) return res;
            int keywordLine = _line;
            res <<= change_list(*lists[i]);
            if(_line == keywordLine) {
                res <<= space(0);
                res <<= keyword(keywords[i]);
            }
            if(res.isError()) return res;
        }

        res <<= _world->addAction(
                actionName, actionGroups, 
                isNotApplicable, arguments, 
                preList, remList, addList);

        for(StrVec::size_type i = 0; i < quests.size(); ++i)
            if(parentQuests.find(quests[i]) == parentQuests.end()) {
                // status change command without PARENT
                res <<= _world->addActionQuestStatusChange(
                        actionName, quests[i], statusList[i], goals[i]);
            } else {
                // status change command with PARENT
                res <<= _world->addActionQuestStatusChange(
                        actionName, quests[i], statusList[i], goals[i],
                        parentQuests[quests[i]], parentGoals[quests[i]]);
            }

        if(res.isError())
            res <<= errorParserWorldError(
                    _file, commandLine, _col, _world->getServerWorldName());
        return res;
    }

    /// @brief Parses a quest definition.
    /// @param isMainQuest Is this quest is a main quest.
    /// @return Returns 'Result::OK()' if the reading operation was successful.
    ///         Otherwise, return a detailed error message.
    Result quest_definition(bool isMainQuest) noexcept {
        Str questName;
        Result res;

        res <<= name(questName, UPPER);
        res <<= colon_with_spaces();
        if(res.isError())
            return res;

        const int commandLine = _line;
        res <<= next_line();
        if(res.isError())
            return res;

        // Parse quest options.
        int spaceLimit = -1;
        int searchLimit = -1;
        int omega = -1;
        bool setHeuristic = false;
        bool setStrategy = false;
        bool useActionTree = false;
        QuestHeuristic heuristic = QuestHeuristic::SIMPLE;
        QuestSearchStrategy strategy = QuestSearchStrategy::ASTAR;
        res <<= empty_lines();
        res <<= space(1);
        if(keyword(KEYWORD_OPTIONS).isOk()) {
            // Keyword "options:" was recognized.
            res <<= colon_with_spaces();
            res <<= next_line();
            while(true) {
                if(res.isError())
                    break;
                res <<= empty_lines();
                res <<= space(1);
                Str optionName;
                res <<= name(optionName, LOWER);
                if(optionName == KEYWORD_SEARCH_LIMIT) {
                    res <<= space(1);
                    res <<= pos_int(searchLimit);
                } else if(optionName == KEYWORD_SPACE_LIMIT) {
                    res <<= space(1);
                    res <<= pos_int(spaceLimit);
                } else if(optionName == KEYWORD_OMEGA) {
                    res <<= space(1);
                    res <<= pos_int(omega);
                } else if(optionName == KEYWORD_HEURISTIC) {
                    res <<= space(1);
                    Str heuristicName;
                    res <<= name(heuristicName, UPPER);
                    if(heuristicName == KEYWORD_SIMPLE) {
                        heuristic = QuestHeuristic::SIMPLE;
                        setHeuristic = true;
                    } else if (heuristicName == KEYWORD_HSP) {
                        heuristic = QuestHeuristic::HSP;
                        setHeuristic = true;
                    } else {    
                        res <<= errorParserError(_file, _line, _col, 
                            "Unknown heuristic name '" + heuristicName + "'");
                    }
                } else if(optionName == KEYWORD_STRATEGY) {
                    res <<= space(1);
                    Str strategyName;
                    res <<= name(strategyName, UPPER);
                    if(strategyName == KEYWORD_ASTAR) {
                        strategy = QuestSearchStrategy::ASTAR;
                        setStrategy = true;
                    } else if (strategyName == KEYWORD_DFS) {
                        strategy = QuestSearchStrategy::DFS;
                        setStrategy = true;
                    } else {    
                        res <<= errorParserError(_file, _line, _col, 
                            "Unknown strategy name '" + strategyName + "'");
                    }
                } else if (optionName == KEYWORD_USE_ATREE) {
                    useActionTree = true;
                } else if(optionName == KEYWORD_PRECONDITIONS) {
                    // This is the end of options list.
                    _pos -= _col;
                    _col = 0;
                    res <<= space(1);
                    break;
                } else {
                    // Unknown option
                    res <<= errorParserError(_file, _line, _col, 
                            "Unknown option '" + optionName + "'");
                    break;
                }
                res <<= space(0);
                res <<= next_line();
            }
        }

        if(res.isError())
            return res;
        
        Vector<StrVec> preconditions;
        res <<= keyword(KEYWORD_PRECONDITIONS);
        res <<= colon_with_spaces();
        res <<= next_line();
        if(res.isError()) return res;
        res <<= empty_lines();
        res <<= change_list(preconditions);
        if(res.isError()) return res;

        Vector<Vector<StrVec>> goals;
        res <<= space(1);
        res <<= keyword(KEYWORD_GOAL);
        if(res.isError()) return res;
        do {
            res <<= colon_with_spaces();
            res <<= next_line();
            res <<= empty_lines();
            if(res.isError()) return res;
            
            Vector<StrVec> goal;
            res <<= change_list(goal);
            res <<= empty_lines();
            res <<= space(1);
            if(res.isError())
                return res;
            goals.push_back(goal);
        } while (keyword(KEYWORD_GOAL).isOk());

        StrVec actions;
        res <<= keyword(KEYWORD_ACTIONS);
        res <<= colon_with_spaces();
        res <<= next_line();
        if(res.isError()) return res;
        res <<= name_list(actions, BOTH, "-");

        StrVec objects;
        res <<= empty_lines();
        res <<= space(1);
        res <<= keyword(KEYWORD_OBJECTS);
        res <<= colon_with_spaces();
        res <<= next_line();
        if(res.isError()) return res;
        res <<= name_list(objects, BOTH);

        StrVec subquests;
        res <<= empty_lines();
        res <<= space(1);
        res <<= keyword(KEYWORD_SUBQUESTS);
        res <<= colon_with_spaces();
        res <<= next_line();
        if(res.isError()) return res;
        res <<= name_list(subquests, UPPER);

        res <<= _world->addQuest(
                questName, isMainQuest, preconditions, goals, 
                actions, objects, subquests, useActionTree);
        
        // Setup quest options.
        if(searchLimit >= 0)
            res <<= _world->setQuestOption(
                    questName, QUEST_OPTION_SEARCH_LIMIT, searchLimit);
        if(spaceLimit >= 0)
            res <<= _world->setQuestOption(
                    questName, QUEST_OPTION_SPACE_LIMIT, spaceLimit);
        if(omega >= 0)
            res <<= _world->setQuestOption(
                    questName, QUEST_OPTION_OMEGA, omega);
        if(setHeuristic)
            res <<= _world->setQuestOption(
                    questName, QUEST_OPTION_HEURISTIC, heuristic);
        if(setStrategy)
            res <<= _world->setQuestOption(
                    questName, QUEST_OPTION_STRATEGY, strategy);

        if(res.isError())
            res <<= errorParserWorldError(
                    _file, commandLine, _col, _world->getServerWorldName());
        return res;
    }

    /// @brief Parses a .quest file.
    /// @return Returns 'Result::OK()' if the reading operation was successful.
    ///         Otherwise, return a detailed error message.
    Result parse() noexcept {
        Result res;

        // Parse the source.

        // Version.
        res <<= empty_lines();
        res <<= version();
        res <<= empty_lines();
        if(res.isError())
            return res;

        // Project name.
        res <<= project();
        res <<= empty_lines();
        if(res.isError())
            return res;
        
        while(_src[_pos] != '\0') {
            Str nextCommand;
            const int keywordCol = _col;
            res <<= space(0);
            res <<= name(nextCommand, LOWER);
            res <<= space(0);
            if (res.isError())
                return res;
            
            if(nextCommand == KEYWORD_TYPE)
                res <<= type_definition();
            else if(nextCommand == KEYWORD_OBJECT)
                res <<= object_definition();
            else if(nextCommand == KEYWORD_REL)
                res <<= relation_definition();
            else if(nextCommand == KEYWORD_RLIST)
                res <<= rlist_definition();
            else if(nextCommand == KEYWORD_AGROUP)
                res <<= action_group_definition();
            else if(nextCommand == KEYWORD_ACTION)
                res <<= action_definition();
            else if(nextCommand == KEYWORD_QUEST)
                res <<= quest_definition(false);
            else if(nextCommand == KEYWORD_MAIN_QUEST)
                res <<= quest_definition(true);
            else // Unknown or unsupported keyword.
                return errorInvalidKeyword(_file, _line, keywordCol, nextCommand);

            if(res.isError())
                return res;

            res <<= empty_lines();
        }
        
        return res;
    }
};

/// @brief Parses `.quest` file into a quest world.
Result parseQuestFile(
        World* world, 
        const Str& file, 
        const Str& projectSrc
        ) noexcept {
    QuestProjectParser parser(world, file, projectSrc);
    return parser.parse();
}

Result addFromProjectSRC(
        World* world, 
        const Str& projectFileName, 
        const Str& projectSrc
        ) noexcept {
    return parseQuestFile(world, projectFileName, projectSrc);
}

}
