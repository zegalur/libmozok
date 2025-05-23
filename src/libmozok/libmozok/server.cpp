// Copyright 2024-2025 Pavlo Savchuk. Subject to the MIT license.

#include <libmozok/message_processor.hpp>
#include <libmozok/public_types.hpp>
#include <libmozok/server.hpp>

#include <libmozok/private_types.hpp>
#include <libmozok/error_utils.hpp>
#include <libmozok/message_queue.hpp>
#include <libmozok/world.hpp>
#include <libmozok/script.hpp>

namespace mozok {

Server::~Server() = default;

struct ApplyCommand {
    const Str worldName;
    const Str actionName;
    const StrVec actionArguments;
    const int data;
};

class ServerImpl : public mozok::Server {
    Str _serverName;
    UnorderedMap<Str, UniquePtr<World>> _worlds;
    MessageQueue _messageQueue;

    Queue<ApplyCommand> _actionQueue;
    Mutex _actionQueueMutex;
    ConditionVariable _actionQueueCV;

    Thread _worker;
    Atomic<bool> _isWorkerRunning;
    Atomic<bool> _stopWorkerThread;
    Atomic<bool> _isWorkerJoined;


    explicit ServerImpl(Str serverName) noexcept : 
        _serverName(std::move(serverName)),
        _worlds(),
        _messageQueue(),
        _actionQueue(),
        _isWorkerRunning(false),
        _stopWorkerThread(false),
        _isWorkerJoined(true)
    { /* empty */ }


    void workerFunc() noexcept {
        while(true) {
            // Start with a planning.
            performPlanningUnsafe();

            UniqueLock queueLock(_actionQueueMutex);

            // Wait for the next action.
            _actionQueueCV.wait_for(
                    queueLock, ONE_QUEST_TICK, 
                    [&]{return _actionQueue.empty() == false;});

            if(_actionQueue.empty()) {
                // _actionQueue is still empty.
                if(_stopWorkerThread.load() == true) {
                    // Stopping of the worker was requested, and now, once 
                    // the queue is empty, we can stop the worker.
                    queueLock.unlock();
                    break;
                } else {
                    // Wait for the next event.
                    queueLock.unlock();
                    continue;
                }
            }

            // Read the next apply action command.
            ApplyCommand nextCommand = _actionQueue.front();
            _actionQueue.pop();
            queueLock.unlock();

            ActionError actionError = MOZOK_AE_NO_ERROR;
            Result res = applyActionUnsafe(
                    nextCommand.worldName,
                    nextCommand.actionName,
                    nextCommand.actionArguments,
                    actionError);
            if(res.isError())
                _messageQueue.onActionError(
                    nextCommand.worldName,
                    nextCommand.actionName, 
                    nextCommand.actionArguments,
                    res, 
                    actionError,
                    nextCommand.data);
        }

        _isWorkerRunning.store(false);
    }

    Result applyActionUnsafe(
        const Str& worldName,
        const Str& actionName,
        const StrVec& actionArguments,
        ActionError& actionError
        ) noexcept {
        if(hasWorld(worldName) == false)
            return errorWorldDoesntExist(_serverName, worldName);
        return _worlds[worldName]->applyAction(
                actionName, actionArguments, _messageQueue, actionError);
    }

    void performPlanningUnsafe() noexcept {
        for(auto& worlds : _worlds)
            worlds.second->performPlanning(_messageQueue);
    }

public:
    ~ServerImpl() noexcept override {
        if(_isWorkerJoined.load() == false) {
            _stopWorkerThread.store(true);
            _worker.join();
        }
    }

    static Server* createServerImpl(
            Str serverName,
            Result& outResult
            ) noexcept {
        outResult = Result::OK();
        return new ServerImpl(std::move(serverName));
    }

    // =============================== WORLD ================================ //

    Result createWorld(const Str& worldName) noexcept override {
        if(_isWorkerJoined.load() == false)
            return errorServerWorkerIsRunning(_serverName);
        if(hasWorld(worldName))
            return errorWorldAlreadyExists(_serverName, worldName);
        _worlds[worldName] = makeUnique<World>(_serverName, worldName);
        return {};
    }

    Result deleteWorld(const Str& worldName) noexcept override {
        if(_isWorkerJoined.load() == false)
            return errorServerWorkerIsRunning(_serverName);
        if(!hasWorld(worldName))
            return errorWorldDoesntExist(_serverName, worldName);
        _worlds.erase(worldName);
        return {};
    }

    bool hasWorld(const Str& worldName) const noexcept override {
        return (_worlds.find(worldName) != _worlds.end());
    }

    StrVec getWorlds() const noexcept override {
        StrVec r;
        for(const auto& it : _worlds)
            r.push_back(it.first);
        return r;
    }

    // ============================== PROJECT =============================== //

    Result addProject(
            const Str& worldName,
            const Str& projectFileName,
            const Str& projectSrc
            ) noexcept override {
        if(_isWorkerJoined.load() == false)
            return errorServerWorkerIsRunning(_serverName);
        if(!hasWorld(worldName))
            return errorWorldDoesntExist(_serverName, worldName);
        return _worlds[worldName]->addProject(projectFileName, projectSrc);
    }

    Result tryProject(
            const Str& /*worldName*/,
            const Str& /*projectFileName*/,
            const Str& /*projectSrc*/
            ) noexcept override {
        if(_isWorkerJoined.load() == false)
            return errorServerWorkerIsRunning(_serverName);
        return errorNotImplemented(__FILE__, __LINE__, __FUNCTION__);
    }

    // ============================== PROJECT =============================== //
    
    Result loadQuestScriptFile(
            FileSystem* fileSystem,
            const Str& scriptFileName,
            const Str& scriptSrc,
            bool applyInitActions
            ) noexcept override {
        return QuestScriptParser_Base::parseHeader(
                this, fileSystem, scriptFileName, scriptSrc, applyInitActions);
    }


    // ============================== OBJECTS =============================== //
    
    bool hasObject(
            const mozok::Str& worldName,
            const mozok::Str& objectName
            ) noexcept override {
        if(hasWorld(worldName) == false)
            return false;
        return _worlds[worldName]->hasObject(objectName);
    }

    StrVec getObjects(
            const Str& worldName
            ) const noexcept override {
        if(hasWorld(worldName) == false)
            return {};
        return _worlds.find(worldName)->second->getObjects();
    }

    StrVec getObjectType(
            const Str& worldName,
            const Str& objectName
            ) const noexcept override {
        if(hasWorld(worldName) == false)
            return {};
        return _worlds.find(worldName)->second->getObjectType(objectName);
    }

    // =============================== QUESTS =============================== //
    
    bool hasSubQuest(
            const mozok::Str& worldName,
            const mozok::Str& subQuestName
            ) noexcept override {
        if(hasWorld(worldName) == false)
            return false;
        return _worlds[worldName]->hasSubquest(subQuestName);
    }

    bool hasMainQuest(
            const mozok::Str& worldName,
            const mozok::Str& mainQuestName
            ) noexcept override {
        if(hasWorld(worldName) == false)
            return false;
        return _worlds[worldName]->hasMainQuest(mainQuestName);
    }

    
    // ============================== ACTIONS =============================== //

    Result applyAction(
            const Str& worldName,
            const Str& actionName,
            const StrVec& actionArguments,
            ActionError& actionError
        ) noexcept override {
        if(_isWorkerJoined.load() == false)
            return errorServerWorkerIsRunning(_serverName);
        return applyActionUnsafe(
                worldName, actionName, actionArguments, actionError);
    }

    Result pushAction(
            const Str& worldName,
            const Str& actionName,
            const StrVec& actionArguments,
            const int data
        ) noexcept override {
        if(hasWorld(worldName) == false)
            return errorWorldDoesntExist(_serverName, worldName);
        _actionQueueMutex.lock();
        _actionQueue.push({worldName, actionName, actionArguments, data});
        _actionQueueMutex.unlock();
        _actionQueueCV.notify_all();
        return Result::OK();
    }

    Server::ActionStatus getActionStatus(
            const Str& worldName,
            const Str& actionName
            ) const noexcept override {
        if(hasWorld(worldName) == false)
            return ACTION_UNDEFINED;
        if(_worlds.find(worldName)->second->hasAction(actionName) == false)
            return ACTION_UNDEFINED;
        if(_worlds.find(worldName)->second->isActionNotApplicable(actionName))
            return ACTION_NOT_APPLICABLE;
        return ACTION_APPLICABLE;
    }

    Result checkAction(
            const bool doNotCheckPreconditions,
            const Str& worldName,
            const Str& actionName,
            const StrVec& arguments
            ) const noexcept override {
        if(hasWorld(worldName) == false)
            return errorWorldDoesntExist(_serverName, worldName);
        return _worlds.find(worldName)->second->checkAction(
                doNotCheckPreconditions, actionName, arguments);
    }

    StrVec getActions(
            const mozok::Str& worldName
            ) const noexcept override {
        if(hasWorld(worldName) == false)
            return {};
        const auto& it = _worlds.find(worldName);
        return it->second->getActions();
    }

    Vector<StrVec> getActionType(
            const Str& worldName,
            const Str& actionName
            ) const noexcept override {
        if(hasWorld(worldName) == false)
            return {};
        const auto& it = _worlds.find(worldName);
        return it->second->getActionType(actionName);
    }

    // ============================== MESSAGES ============================== //

    bool processNextMessage(
            MessageProcessor& messageProcessor) noexcept override {
        return _messageQueue.processNext(messageProcessor);
    }

    // ============================== PLANNING ============================== //

    Result performPlanning() noexcept override {
        if(_isWorkerJoined.load() == false)
            return errorServerWorkerIsRunning(_serverName);
        performPlanningUnsafe();
        return Result::OK();
    }

    // =============================== WORKER =============================== //

    Result startWorkerThread() noexcept override {
        if(_isWorkerJoined.load() == false)
            return errorServerWorkerIsRunning(_serverName);
        _stopWorkerThread.store(false);
        _isWorkerRunning.store(true);
        _isWorkerJoined.store(false);
        _worker = Thread(&ServerImpl::workerFunc, this);
        return Result::OK();
    }

    bool stopWorkerThread() noexcept override {
        _stopWorkerThread.store(true);
        if(_isWorkerRunning.load() == true)
            return false;
        _worker.join();
        _isWorkerJoined.store(true);
        return true;
    }

    // =============================== SAVING =============================== //

    Str generateSaveFile(const Str& worldName) noexcept override {
        if(_isWorkerJoined.load() == false)
            return "error: Doesn't allowed while worker thread is running.";
        if(!hasWorld(worldName))
            return "error: Undefined world '" + worldName + "'.";
        if(_messageQueue.size() != 0)
            return "error: Doesn't allowed while message queue isn't empty.";
        return _worlds[worldName]->generateSaveFile();
    }

};


Server* Server::createServer(
        const Str serverName,
        Result& outResult) noexcept {
    return ServerImpl::createServerImpl(std::move(serverName), outResult);
}


}
