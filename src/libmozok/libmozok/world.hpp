// Copyright 2024-2025 Pavlo Savchuk. Subject to the MIT license.

#pragma once

#include <libmozok/public_types.hpp>
#include <libmozok/message_processor.hpp>
#include <libmozok/private_types.hpp>
#include <libmozok/result.hpp>

#include <libmozok/type.hpp>
#include <libmozok/object.hpp>
#include <libmozok/relation.hpp>
#include <libmozok/relation_list.hpp>
#include <libmozok/action.hpp>
#include <libmozok/quest.hpp>

#include <libmozok/state.hpp>
#include <libmozok/quest_manager.hpp>

namespace mozok {

/// @brief A world is a collection of types, objects, relations, relation lists, 
///        actions, and quests. Together with a current game world state, they 
///        form the complete model of a quest world.
/// To utilize the libmozok quest system, you must first construct your game 
/// world rules by defining types, objects, relations, actions, and quests. 
/// A specific state of a quest world is modeled as a list of interconnected 
/// objects linked through relations. A quest is described as a list of goals 
/// and preconditions. Finally, through actions, quest objectives are 
/// accomplished by logically altering the state of the world.
class World {

    /// @brief The name of the server to which this world belongs.
    const Str _serverName;
    
    /// @brief The name of the world.
    const Str _worldName;

    /// @brief Combined `ServerName:WorldName` string.
    const Str _serverWorldName;

    /// @brief The state of the world.
    StatePtr _state;

    /// @brief Starting from 0, each change in the state increments this ID by 1.
    ID _stateId;

    //==========================================================================
    /// @defgroup Parts of the world
    /// @{

    /// @brief The list of all defined types within this world.
    TypeVec _types;

    /// @brief The list of all defined objects within this world.
    ObjectVec _objects;

    /// @brief The list of all defined relations within this world.
    RelationVec _relations;

    /// @brief The list of all defined relation lists within this world.
    RelationListVec _relationLists;

    /// @brief The list of all defined world actions.
    ActionVec _actions;

    /// @brief The list of all defined main quests.
    QuestManagerVec _mainQuests;

    /// @brief The list of all defined subquests.
    QuestManagerVec _subquests;


    /// @brief Maps the name of a type to its corresponding index in the 
    ///        `_types` array. This index serves as the types's unique 
    ///        non-negative identifier."
    UnorderedMap<Str, ID> _typeNameToId;

    /// @brief Maps the name of an object to its corresponding index in the 
    ///        `_objects` array. This index serves as the object's unique 
    ///        non-negative identifier."
    UnorderedMap<Str, ID> _objectNameToId;

    /// @brief Maps the name of a relation to its corresponding index in the 
    ///        `_relations` array. This index serves as the relation's unique 
    ///        non-negative identifier."
    UnorderedMap<Str, ID> _relationNameToId;

    /// @brief Maps the name of a relation list to its corresponding index in 
    ///        the `_relationLists` array. This index serves as the rlist's 
    ///        unique non-negative identifier."
    UnorderedMap<Str, ID> _relationListNameToId;

    /// @brief Maps the name of an action to its corresponding index in the 
    ///        `_actions` array. This index serves as the action's unique 
    ///        non-negative identifier."
    UnorderedMap<Str, ID> _actionNameToId;

    /// @brief Maps the name of a main quest to its corresponding index in the 
    ///        `_mainQuests` array. This index serves as the quest's unique 
    ///        non-negative identifier."
    UnorderedMap<Str, ID> _mainQuestNameToId;

    /// @brief Maps the name of a subquest to its corresponding index in the 
    ///        `_subquests` array. This index serves as the quest's unique 
    ///        non-negative identifier."
    UnorderedMap<Str, ID> _subquestNameToId;

    const TypePtr& getType(const Str& typeName) const noexcept;
    const ObjectPtr& getObject(const Str& objectName) const noexcept;
    const RelationPtr& getRelation(const Str& relationName) const noexcept;
    const RelationListPtr& getRelationList(const Str& rlistName) const noexcept;
    const ActionPtr& getAction(const Str& actionName) const noexcept;

    QuestManagerPtr& getMainQuest(const Str& questName) noexcept;
    QuestManagerPtr& getSubquest(const Str& questName) noexcept;
    const QuestManagerPtr& getMainQuest(const Str& questName) const noexcept;
    const QuestManagerPtr& getSubquest(const Str& questName) const noexcept;

    /// @}


    //==========================================================================
    /// @defgroup Special methods
    /// @{

    /// @brief This method constructs the "full type" for a given list of type 
    ///        names. The "full type" represents all possible types that an 
    ///        object described by the given list could belong to.
    /// @param types A list of types, should contain only a known type names.
    /// @param out The resulting "full type" will be added to this set.
    /// @return Returns the status of the operation.
    Result constructFullType(const StrVec& types, TypeSet& out) const noexcept;

    /// @brief For a given list of known type names this method will output the
    ///        the list of corresponding type objects.
    /// @param types A list type names, should contain only a known type names.
    /// @param out The resulting list will be added to this vector.
    /// @return Returns the status of the operation.
    Result constructTypeVec(const StrVec& types, TypeVec& out) const noexcept;

    /// @brief For a provided list of argument names and type names, this method 
    /// generates a series of new objects with corresponding types and negative 
    /// identifiers (commencing from 1, where the i-th argument will have an 
    /// identifier equal to (-i)).
    /// @param arguments [["arg1Name", "Type1Name", "Type2Name", ... ], ... ]
    /// @param out The resulting objects will be added to this vector.
    /// @return Returns the status of the operation.
    Result constructArguments(
                const Vector<StrVec> &arguments, ObjectVec& out) const noexcept;

    /// @brief For a provided list of relation and local object names, this 
    ///        method generates a series of corresponding statements.
    /// @param list [["RelationName", "obj1Name", "obj2Name", ... ], ... ]
    /// @param localObjects A collection of local objects (variables) that 
    ///                     can be referenced within the provided list.
    /// @param out The resulting objects will be added to this vector.
    /// @return Returns the status of the operation.
    Result constructStatements(
                const Vector<StrVec> &list, 
                const ObjectVec& localObjects, 
                StatementVec& out
                ) const noexcept;

    /// @}

    /// @brief Action's status change command.
    struct QuestStatusChangeCommand {
        /// @brief A quest whose status will be changed.
        QuestManagerPtr quest;
        /// @brief A new status.
        const QuestStatus status;
        /// @brief A new goal.
        const int goal;
        /// @brief Parent quest name (non-empty for subquests)
        const QuestManagerPtr parentQuest;
        /// @brief Parent quest goal associated with this subquest.
        const int parentGoal;
    };
    using QuestStatusChangeCommandMap = 
                UnorderedMap<ID, Vector<QuestStatusChangeCommand>>;
    /// @brief Describes actions status change commands.
    QuestStatusChangeCommandMap _actionStatusChangeCommands;

public:
    World(const Str& serverName, const Str& worldName) noexcept;

    /// @brief Returns the name of the server where this world was created.
    const Str& getServerName() const noexcept;

    /// @brief Returns the name of the quest world. 
    const Str& getWorldName() const noexcept;

    /// @brief Returns the combined `ServerName:WorldName`.
    const Str& getServerWorldName() const noexcept;

    /// @brief Generates a .quest file with the current world state. See 
    ///        `Server::generateSaveFile()` for more details.
    /// @see Server::generateSaveFile()
    Str generateSaveFile() noexcept;

    //==========================================================================
    /// @defgroup Projects
    /// @{

    /// @brief Adds a project to the world. See Server::addProject() 
    ///        for more details.
    /// @see Server::addProject()
    /// @return Returns the status of the operation.
    Result addProject(
            const Str& projectFileName,
            const Str& projectSrc
            ) noexcept;

    /// @}

    // =============================== TYPE ================================= //

    /// @brief Defines a new type.
    /// @param typeName Type's unique name.
    /// @param supertypeNames A list of previously defined type names.
    /// @return Returns the status of the operation.
    Result addType(
            const Str& typeName,
            const StrVec& supertypeNames = {}
            ) noexcept;

    bool hasType(const Str& typeName) const noexcept;

    // ============================== OBJECT ================================ //

    /// @brief Defines a new object.
    /// @param objectName Objects's unique name.
    /// @param typeNames A list of previously defined type names.
    /// @return Returns the status of the operation.
    Result addObject(
            const Str& objectName,
            const StrVec& typeNames
            ) noexcept;

    /// @brief Checks if the world has an object with a give name.
    /// @param objectName Object name.
    /// @return Returns `true` if an object with a given name is defined.
    ///         Returns `false` if no object with such a name was defined.
    bool hasObject(const Str& objectName) const noexcept;

    /// @brief Returns a list of all objects from this world.
    StrVec getObjects() const noexcept;

    /// @brief Returns a list of all types that this object inherits from 
    ///        or can be considered as. If object doesn't exists,
    ///        returns an empty array.
    StrVec getObjectType(
            const Str& objectName
            ) const noexcept;

    // ============================= RELATION =============================== //

    /// @brief Defines a new relation.
    /// @param relationName Relation's unique name.
    /// @param argumentTypeNames A list of previously defined type names.
    /// @return Returns the status of the operation.
    Result addRelation(
            const Str& relationName,
            const StrVec& argumentTypeNames = {}
            ) noexcept;

    /// @brief Checks if the world has a relation with a give name.
    /// @param relationName Relation name.
    /// @return Returns `true` if a relation with a given name is defined.
    ///         Returns `false` if no relation with such a name was defined.
    bool hasRelation(const Str& relationName) const noexcept;

    // ========================== RELATION LIST ============================= //

    /// @brief Defines a new relation list.
    /// @param relationListName Relation list's unique name.
    /// @param arguments A list of rlist arguments in this format: 
    ///         [ ["argName", "TypeName", "TypeName", ...], ... ].
    /// @param list A list of rlist statements in this format: 
    ///         [ ["RelOrRListName", "objOrArgName", "objOrArgName", ...], ... ].
    /// @return Returns the status of the operation.
    Result addRelationList(
            const Str& relationListName,
            const Vector<StrVec> &arguments,
            const Vector<StrVec> &list
            ) noexcept;

    /// @brief Checks if the world has a relation list with a give name.
    /// @param relationListName Relation list name.
    /// @return Returns `true` if a relation list with a given name is defined.
    ///         Returns `false` if no relation list with such a name was defined.
    bool hasRelationList(const Str& relationListName) const noexcept;
        
    // ============================== ACTION ================================ //

    /// @brief Defines a new action.
    /// @param actionName Action's unique name.
    /// @param isNotApplicable Is this action not applicable. Not applicable 
    ///         actions cannot be applied with 'World::applyAction()'.
    /// @param arguments A list of action arguments in this format: 
    ///         [ ["argName", "TypeName", "TypeName", ...], ... ].
    /// @param preList A list of action "pre" statements in this format: 
    ///         [ ["RelOrRListName", "objOrArgName", "objOrArgName", ...], ... ].
    /// @param remList A list of action "rem" statements in this format: 
    ///         [ ["RelOrRListName", "objOrArgName", "objOrArgName", ...], ... ].
    /// @param addList A list of action "add" statements in this format: 
    ///         [ ["RelOrRListName", "objOrArgName", "objOrArgName", ...], ... ].
    /// @return Returns the status of the operation.
    Result addAction(
            const Str& actionName,
            const bool isNotApplicable,
            const Vector<StrVec> &arguments,
            const Vector<StrVec> &preList,
            const Vector<StrVec> &remList,
            const Vector<StrVec> &addList
            ) noexcept;

    /// @brief Checks if the world has an action with a give name.
    /// @param actionName Action name.
    /// @return Returns `true` if an action with a given name is defined.
    ///         Returns `false` if no action with such a name was defined.
    bool hasAction(const Str& actionName) const noexcept;

    /// @brief Checks if action with a given name is not applicable. Not 
    ///        applicable actions cannot be applied with 'World::applyAction()'.
    /// @param actionName Action name.
    /// @return Returns `true` if action is undefined or not applicable.
    ///         Returns `false` if action is defined and applicable.
    bool isActionNotApplicable(const Str& actionName) const noexcept;

    /// @brief Applies an action to a world. Then it activates all inactive main 
    ///        quests with now consistent preconditions.
    /// @param actionName The name of the action you want to apply.
    /// @param actionArguments Object names list (will be used as arguments).
    /// @param messageProcessor A message processor.
    /// @param errorOutput Writes action error code in a case of an error.
    /// @return Returns the status of the operation.
    Result applyAction(
        const Str& actionName,
        const StrVec& actionArguments,
        MessageProcessor& messageProcessor,
        ActionError& errorOutput
        ) noexcept;

    /// @brief Checks if action with this set of arguments is well defined.
    ///        Optionally, checks if preconditions hold (in the current state).
    /// @param doNotCheckPreconditions If `true` - skips preconditions check.
    /// @param actionName The name of the action.
    /// @param arguments A list of object names we'll use as arguments.
    /// @return Returns error when action or any other name isn't defined, or
    ///         when arity or types were wrong. 
    Result checkAction(
        const bool doNotCheckPreconditions,
        const Str& actionName,
        const StrVec& actionArguments
        ) const noexcept;

    /// @brief Adds a quest status command to an action.
    /// @param actionName Action name.
    /// @param questName The name of the quest whose status will be changed.
    /// @param status A new status.
    /// @param goal A new goal.
    /// @param parentQuestName Parent quest name (empty for main quests)
    /// @param parentQuestGoal Parent quest goal, associated with the subquest.
    /// @return Returns the status of the operation.
    Result addActionQuestStatusChange(
            const Str& actionName,
            const Str& questName,
            const QuestStatus status,
            int goal,
            const Str& parentQuestName = "",
            int parentQuestGoal = 0
            ) noexcept;
    
    /// @brief Returns a list of world actions.
    StrVec getActions() const noexcept;

    /// @brief Returns a type of an action as a vector of [name, types...]
    ///        If action doesn't exists, returns an empty array.
    Vector<StrVec> getActionType(
        const mozok::Str& actionName
        ) const noexcept;

    // =============================== QUEST ================================ //

    /// @brief Defines a new quest.
    /// @param questName Quest's unique name.
    /// @param isMainQuest Is this quest is a main quest.
    /// @param preconditions Quest preconditions statements in this format: 
    ///         [ ["RelOrRListName", "objectName", "objectName", ...], ... ].
    /// @param goals Quest goals statements in this format: 
    ///         [ (goal_1) [ [A,obj1,...], [B,obj2,...], ... ],
    ///           (goal_2) [ [A,obj3,...], [B,obj4,...], ... ],
    ///           ... ].
    /// @param questActionNames The list of previously defined allowed actions.
    /// @param questObjectNames The list of previously defined relevant objects.
    ///         A type name will include every known object of this type.
    /// @param questSubquestNames The list of previously defined subquest names.
    /// @param useActionTree If `true`, force to use action tree.
    /// @return Returns the status of the operation.
    Result addQuest(
            const Str& questName,
            const bool isMainQuest,
            const Vector<StrVec> &preconditions,
            // [ goal_1: [ [A,obj1,...], [B,obj2,...] ], 
            //   goal_2: [ [C,obj3,...], [D,obj4,...] ],
            //   ... ]
            const Vector<Vector<StrVec>> &goals,
            const StrVec& questActionNames,
            const StrVec& questObjectNames,
            const StrVec& questSubquestNames,
            const bool useActionTree
            ) noexcept;

    bool hasSubquest(const Str& questName) const noexcept;
    bool hasMainQuest(const Str& questName) const noexcept;

    /// @brief Returns the current status of a quest.
    /// @param questName The name of the quest.
    /// @return Returns quest status. 
    ///         If quest is undefined returns `MOZOK_QUEST_STATUS_INACTIVE`.
    QuestStatus getQuestStatus(const Str& questName) const noexcept;

    /// @brief Sets a quest option.
    /// @param questName The name of a previously defined quest.
    /// @param option Quest option.
    /// @param value Option value.
    /// @return Returns the status of the operation.
    Result setQuestOption(
            const Str& questName, 
            const QuestOption option, 
            const int value
            ) noexcept;

private:

    /// @brief Activates currently inactive main quest.
    /// @param messageProcessor A message processor that will receive 
    ///         `onNewMainQuest()` message.
    void activateInactiveMainQuests(MessageProcessor& messageProcessor) noexcept;

    /// @brief Attempts to find a subquest for a newly created quest plan.
    /// @param questManager A quest with a new plan.
    /// @param messageProcessor A message processor that will receive 
    ///         `onNewSubquest()` message.
    void findNewSubquest(
                QuestManagerPtr& questManager,
                MessageProcessor& messageProcessor
                ) noexcept;

    void performQuestPlanning(
                QuestManagerPtr& questManager,
                MessageProcessor& messageProcessor
                ) noexcept;
public:

    // ============================= PLANNING =============================== //

    /// @brief Performs planning for all active quests.
    /// @param messageProcessor A message processor.
    void performPlanning(
            MessageProcessor& messageProcessor
            ) noexcept;


};

}
