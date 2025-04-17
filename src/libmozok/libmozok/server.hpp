// Copyright 2024 Pavlo Savchuk. Subject to the MIT license.

#pragma once

#include <libmozok/public_types.hpp>
#include <libmozok/result.hpp>
#include <libmozok/message_processor.hpp>
#include <libmozok/filesystem.hpp>

namespace mozok {

/// @brief Server serves as the designated public API for accessing libmozok. 
/// All interactions between end-user (such as engine, program, etc.) and 
/// libmozok must be exclusively routed through this class. Instead of employing 
/// the exceptions, libmozok utilizes a specialized @ref Result structure to 
/// communicate both successful outcomes and encountered errors.
/// @see Result
class Server {
public:
    virtual ~Server();

    /// @brief Creates a new Quests Server with the specified name.
    /// @param serverName Server name.
    /// @param outResult Used to return the status of the operation.
    /// @return Returns a pointer to the newly create Quests Server. 
    ///         In a case of an error, this static method will return a nullptr.
    static mozok::Server* createServer(
        const mozok::Str serverName,
        mozok::Result& outResult
        ) noexcept;


    //==========================================================================
    /// @defgroup World
    /// @{

    /// @brief Creates a new quests world with the specified name.
    /// @param worldName World name.
    /// @return Returns the status of the operation.
    /// @see addProject()
    virtual mozok::Result createWorld(const mozok::Str& worldName) noexcept = 0;

    /// @brief Removes a world.
    /// @param worldName The name of the world you wish to remove.
    /// @return Returns the status of the operation.
    virtual mozok::Result deleteWorld(const mozok::Str& worldName) noexcept = 0;

    /// @brief Checks if a world with the specified name exists.
    /// @param worldName The name of the world.
    /// @return Returns `true` if a world with a specified name exists. 
    ///         Otherwise, returns false.
    virtual bool hasWorld(const mozok::Str& worldName) const noexcept = 0;

    /// @}


    //==========================================================================
    /// @defgroup Project
    /// @{

    /// @brief Adds a project to the world.
    /// Project is a collections of types, objects, relations, relation lists,
    /// actions and quests, which collectively form a cohesive and logical whole.
    /// WARNING! This operation does not function as a transaction. An error during 
    /// the addition process could result in an incomplete game world.
    /// @param worldName The name of the world to which the project 
    ///         needs to be added.
    /// @param projectFileName The project file name. 
    ///         Utilized only for error handling purposes.
    /// @param projectSrc Project source code in the .quest format.
    /// @return Returns the status of the operation.
    virtual mozok::Result addProject(
            const mozok::Str& worldName,
            const mozok::Str& projectFileName,
            const mozok::Str& projectSrc
            ) noexcept = 0;

    /// @brief Tries a project file.
    /// @param worldName The name of the world.
    /// @param projectFileName The project file name. 
    ///         Utilized only for error handling purposes.
    /// @param projectSrc Project source code in the .quest format.
    /// @return Returns the status of the operation.
    virtual mozok::Result tryProject(
            const mozok::Str& worldName,
            const mozok::Str& projectFileName,
            const mozok::Str& projectSrc
            ) noexcept = 0;

    /// @}


    //==========================================================================
    /// @defgroup Script
    /// @{

    /// ...
    virtual mozok::Result loadQuestScriptFile(
            mozok::FileSystem* fileSystem,
            const mozok::Str& scriptFileName,
            const mozok::Str& scriptSrc,
            bool applyInitActions
            ) noexcept = 0;

    /// @}


    //==========================================================================
    /// @defgroup Objects
    /// @{

    /// @brief Checks if a world has an object.
    /// @param worldName The name of the world.
    /// @param objectName The name of the object.
    /// @return Returns `true` if this world has the object. 
    ///         If world or objects doesn't exist, returns false.
    virtual bool hasObject(
            const mozok::Str& worldName,
            const mozok::Str& objectName
            ) noexcept = 0;

    /// @}
    
    
    //==========================================================================
    /// @defgroup Quests
    /// @{

    /// @brief Checks if a world has a main quest with a specific name.
    /// @param worldName The name of the world.
    /// @param mainQuestName The name of the main quest.
    /// @return Returns `true` if this world has the main quest. 
    ///         If world or main quest doesn't exist, returns false.
    virtual bool hasMainQuest(
            const mozok::Str& worldName,
            const mozok::Str& mainQuestName
            ) noexcept = 0;

    /// @}

    /// @brief Checks if a world has a subquest with a specific name.
    /// @param worldName The name of the world.
    /// @param subQuestName The name of the subquest.
    /// @return Returns `true` if this world has the subquest. 
    ///         If world or main quest doesn't exist, returns false.
    virtual bool hasSubQuest(
            const mozok::Str& worldName,
            const mozok::Str& subQuestName
            ) noexcept = 0;

    /// @}

    //==========================================================================
    /// @defgroup Actions
    /// @{

    /// @brief Action status values.
    enum ActionStatus {
        // Action is undefined or world doesn't exist.
        ACTION_UNDEFINED,

        // Action is defined and applicable.
        ACTION_APPLICABLE,

        // Action is defined and not applicable.
        ACTION_NOT_APPLICABLE
    };

    /// @brief Applies an action to a world. Then it activates all inactive main 
    ///        quests with now consistent preconditions. In a case of an error,
    ///        it will return the error status, but it will not trigger the
    ///        `onActionError` message.
    /// @param worldName The name of the world where action must be applied.
    /// @param actionName The name of the action you want to apply.
    /// @param actionArguments Object names list (will be used as arguments).
    /// @return Returns the status of the operation.
    virtual mozok::Result applyAction(
        const mozok::Str& worldName,
        const mozok::Str& actionName,
        const mozok::StrVec& actionArguments
        ) noexcept = 0;
    
    /// @brief Pushes an action into action queue. Action then will be executed
    ///        by a worker thread.
    /// @param worldName The name of the world where action must be applied.
    /// @param actionName The name of the action you want to apply.
    /// @param actionArguments Object names list (will be used as arguments).
    /// @return Returns the status of the operation.
    virtual mozok::Result pushAction(
        const mozok::Str& worldName,
        const mozok::Str& actionName,
        const mozok::StrVec& actionArguments
        ) noexcept = 0;

    /// @brief Returns the status of the action.
    /// @param worldName The name of the world where we check the status.
    /// @param actionName Action name.
    /// @return Returns the status of the action within a given world.
    virtual mozok::Server::ActionStatus getActionStatus(
        const mozok::Str& worldName,
        const mozok::Str& actionName
        ) const noexcept = 0;

    /// @}


    //==========================================================================
    /// @defgroup Messages
    /// @{

    /// @brief Processes and removes the first message from the queue.
    /// @param messageProcessor A message processor.
    /// @return Returns `false` if no messages left.
    virtual bool processNextMessage(
            mozok::MessageProcessor& messageProcessor) noexcept = 0;

    /// @}


    //==========================================================================
    /// @defgroup Planner
    /// @{

    /// @brief Performs one planning step for all active quests.
    virtual mozok::Result performPlanning() noexcept = 0;

    /// @}


    //==========================================================================
    /// @defgroup Worker
    /// @{

    /// @brief Starts server's worker thread.
    /// @return Returns the status of the operation.
    virtual mozok::Result startWorkerThread() noexcept = 0;

    /// @brief Send a signal to stop the server's worker thread. If there are 
    ///        any unprocessed actions and planning tasks remaining, the worker 
    ///        thread will complete them all before actually stopping.
    /// @return Returns `false` if thread is still running.
    ///         Returns `true` if thread has been stopped.
    virtual bool stopWorkerThread() noexcept = 0;

    /// @}


    //==========================================================================
    /// @defgroup Saving
    /// @{

    /// @brief Generates a .quest file with the current state of the given world.
    /// @param worldName World name.
    /// @return Returns a string containing a valid .quest file with the current
    /// state of the world. In order to load the state, first you need to add
    /// the other projects with all the necessary definitions, then add this 
    /// project and apply `Load()` action. In a case of error returns invalid
    /// .quest file with an error message `error: [...error message...]`.
    virtual mozok::Str generateSaveFile(const mozok::Str& worldName) noexcept = 0;

    /// @}

};

}
