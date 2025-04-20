// Copyright 2024 Pavlo Savchuk. Subject to the MIT license.

#pragma once 

#include <libmozok/public_types.hpp>
#include <libmozok/result.hpp>
#include <libmozok/message_processor.hpp>

#include <libmozok/private_types.hpp>


namespace mozok {

/// @brief A message from `MessageQueue`.
class Message {
protected:
    /// @brief The name of the world from which this message was sent.
    const Str _worldName;
public:
    virtual ~Message();
    Message(const Str& worldName) noexcept;
    
    /// @brief Process the message using the provided processor.
    /// @param messageProcessor A message processor.
    virtual void process(
            MessageProcessor& messageProcessor) const noexcept = 0;
};
using MessagePtr = SharedPtr<const Message>;
using MessageQueueType = Queue<MessagePtr>;


//==========================================================================

/// @brief Thread-safe message queue.
/// Messages can be added to the queue either during planning or after an action 
/// has been applied.
class MessageQueue : public MessageProcessor {
    /// @brief Container for storing unprocessed messages.
    MessageQueueType _queue;
    Mutex _queueMutex;

    void pushMessage(MessagePtr& msg) noexcept;

public:
    MessageQueue() noexcept;

    /// @brief Process all the messages from the queue using the provided processor.
    /// @param processor A message processor.
    void processAll(MessageProcessor& processor) noexcept;

    /// @brief Process the next message from the queue using the provided processor.
    /// @param processor A message processor.
    /// @return Returns `false` if message queue is empty.
    bool processNext(MessageProcessor& processor) noexcept;

    /// @brief Returns the number of unprocessed messages.
    SIZE_T size() noexcept;

    void onActionError(
        const Str& worldName, 
        const Str& actionName,
        const StrVec& actionArguments,
        const Result& errorResult,
        const ActionError actionError,
        const int data
        ) noexcept override;

    void onNewMainQuest(
        const Str& worldName, 
        const Str& questName
        ) noexcept override;

    void onNewSubQuest(
        const Str& worldName, 
        const Str& subquestName,
        const Str& parentQuestName,
        const int goal
        ) noexcept override;
    
    void onNewQuestState(
        const Str& worldName, 
        const Str& questName
        ) noexcept override;
    
    void onNewQuestStatus(
        const Str& worldName, 
        const Str& questName,
        const QuestStatus questStatus
        ) noexcept override;

    void onNewQuestGoal(
        const Str& worldName,
        const Str& questName,
        const int newGoal,
        const int oldGoal
        ) noexcept override;

    void onNewQuestPlan(
        const Str& worldName, 
        const Str& questName,
        const StrVec& actionList,
        const Vector<StrVec>& actionArgsList
        ) noexcept override;

    void onSearchLimitReached(
        const mozok::Str& worldName,
        const mozok::Str& questName,
        const int searchLimitValue
        ) noexcept override;
    
    void onSpaceLimitReached(
        const mozok::Str& worldName,
        const mozok::Str& questName,
        const int spaceLimitValue
        ) noexcept override;
};


//==========================================================================
/// @defgroup Classes for specific messages.
/// @{

class OnActionError : public Message {
    const Str _actionName;
    const StrVec _actionArguments;
    const Result _errorResult;
    const ActionError _actionError;
    const int _data;
public:
    OnActionError(
            const Str& worldName, 
            const Str& actionName,
            const StrVec& actionArguments,
            const Result& errorResult,
            const ActionError actionError,
            const int data
            ) noexcept;
    void process(MessageProcessor& messageProcessor) const noexcept override;
};

class OnNewMainQuest : public Message {
    const Str _questName;
public:
    OnNewMainQuest(
            const Str& worldName, 
            const Str& questName) noexcept;
    void process(MessageProcessor& messageProcessor) const noexcept override;
};


class OnNewSubQuest : public Message {
    const Str _subquestName;
    const Str _parentQuestName;
    const int _goal;
public:
    OnNewSubQuest(
            const Str& worldName, 
            const Str& subquestName,
            const Str& parentQuestName,
            const int goal) noexcept;
    void process(MessageProcessor& messageProcessor) const noexcept override;
};


class OnNewQuestState : public Message {
    const Str _questName;
public:
    OnNewQuestState(
            const Str& worldName, 
            const Str& questName) noexcept;
    void process(MessageProcessor& messageProcessor) const noexcept override;
};


class OnNewQuestStatus : public Message {
    const Str _questName;
    const QuestStatus _status;
public:
    OnNewQuestStatus(
            const Str& worldName, 
            const Str& questName,
            const QuestStatus status) noexcept;
    void process(MessageProcessor& messageProcessor) const noexcept override;
};


class OnNewQuestGoal: public Message {
    const Str _questName;
    const int _newGoal;
    const int _oldGoal;
public:
    OnNewQuestGoal(
            const Str& worldName, 
            const Str& questName,
            const int newGoal,
            const int oldGoal) noexcept;
    void process(MessageProcessor& messageProcessor) const noexcept override;
};


class OnNewQuestPlan : public Message {
    const Str _questName;
    const StrVec _actionList;
    const Vector<StrVec> _actionArgsList;
public:
    OnNewQuestPlan(
            const Str& worldName, 
            const Str& questName,
            const StrVec& actionList,
            const Vector<StrVec>& actionArgsList
            ) noexcept;
    void process(MessageProcessor& messageProcessor) const noexcept override;
};


class OnSearchLimitReached : public Message {
    const Str _questName;
    const int _searchLimitValue;
public:
    OnSearchLimitReached(
            const Str& worldName, 
            const Str& questName,
            const int searchLimitValue
            ) noexcept;
    void process(MessageProcessor& messageProcessor) const noexcept override;
};


class OnSpaceLimitReached : public Message {
    const Str _questName;
    const int _spaceLimitValue;
public:
    OnSpaceLimitReached(
            const Str& worldName, 
            const Str& questName,
            const int spaceLimitValue
            ) noexcept;
    void process(MessageProcessor& messageProcessor) const noexcept override;
};

/// @}

}
