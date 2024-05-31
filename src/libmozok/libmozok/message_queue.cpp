// Copyright 2024 Pavlo Savchuk. Subject to the MIT license.

#include <libmozok/message_queue.hpp>

namespace mozok {


// ================================ MESSAGE ================================= //

Message::~Message() = default;

Message::Message(
        const Str& worldName
        ) noexcept :
    _worldName(worldName) 
{ /* empty */ }


// ============================= MESSAGE QUEUE ============================== //

MessageQueue::MessageQueue() noexcept
{ /* empty */ }

void MessageQueue::processAll(
        MessageProcessor& processor
        ) noexcept {
    while(processNext(processor));
}

bool MessageQueue::processNext(
        MessageProcessor& processor
        ) noexcept {
    _queueMutex.lock();
    if(_queue.empty()) {
        _queueMutex.unlock();
        return false;
    }
    MessagePtr msg = _queue.front();
    _queue.pop();
    _queueMutex.unlock();
    msg->process(processor);
    return true;
}

void MessageQueue::pushMessage(MessagePtr& msg) noexcept {
    _queueMutex.lock();
    _queue.push(msg);
    _queueMutex.unlock();
}

void MessageQueue::onActionError(
        const Str& worldName, 
        const Str& actionName,
        const StrVec& actionArguments,
        const Result& errorResult
        ) noexcept {
    MessagePtr msg = makeShared<OnActionError>(
            worldName, actionName, actionArguments, errorResult);
    pushMessage(msg);
}

void MessageQueue::onNewMainQuest(
        const Str& worldName, 
        const Str& questName
        ) noexcept {
    MessagePtr msg = makeShared<OnNewMainQuest>(
            worldName, questName);
    pushMessage(msg);
}

void MessageQueue::onNewSubQuest(
        const Str& worldName, 
        const Str& subquestName,
        const Str& parentQuestName,
        const int goal
        ) noexcept {
    MessagePtr msg = makeShared<OnNewSubQuest>(
            worldName, subquestName, parentQuestName, goal);
    pushMessage(msg);
}

void MessageQueue::onNewQuestState(
        const Str& worldName, 
        const Str& questName
        ) noexcept {
    MessagePtr msg = makeShared<OnNewQuestState>(
            worldName, questName);
    pushMessage(msg);
}

void MessageQueue::onNewQuestStatus(
        const Str& worldName, 
        const Str& questName,
        const QuestStatus questStatus
        ) noexcept {
    MessagePtr msg = makeShared<OnNewQuestStatus>(
            worldName, questName, questStatus);
    pushMessage(msg);
}

void MessageQueue::onNewQuestPlan(
        const Str& worldName, 
        const Str& questName,
        const StrVec& actionList,
        const Vector<StrVec>& actionArgsList
        ) noexcept {
    MessagePtr msg = makeShared<OnNewQuestPlan>(
            worldName, questName, actionList, actionArgsList);
    pushMessage(msg);
}

void MessageQueue::onSearchLimitReached(
        const mozok::Str& worldName,
        const mozok::Str& questName,
        const int searchLimitValue
        ) noexcept {
    MessagePtr msg = makeShared<OnSearchLimitReached>(
            worldName, questName, searchLimitValue);
    pushMessage(msg);
}
    
void MessageQueue::onSpaceLimitReached(
        const mozok::Str& worldName,
        const mozok::Str& questName,
        const int spaceLimitValue
        ) noexcept {
    MessagePtr msg = makeShared<OnSearchLimitReached>(
            worldName, questName, spaceLimitValue);
    pushMessage(msg);
}


// ============================= MESSAGE LIST =============================== //

OnActionError::OnActionError(
        const Str& worldName, 
        const Str& actionName,
        const StrVec& actionArguments,
        const Result& errorResult
        ) noexcept :
        Message(worldName),
        _actionName(actionName),
        _actionArguments(actionArguments),
        _errorResult(errorResult)
{ /* empty */ }

void OnActionError::process(MessageProcessor& messageProcessor) const noexcept {
    messageProcessor.onActionError(
            _worldName, _actionName, _actionArguments, _errorResult);
}


OnNewMainQuest::OnNewMainQuest(
        const Str& worldName, 
        const Str& questName
        ) noexcept :
    Message(worldName),
    _questName(questName)
{ /* empty */ }

void OnNewMainQuest::process(MessageProcessor& messageProcessor) const noexcept {
    messageProcessor.onNewMainQuest(_worldName, _questName);
}


OnNewSubQuest::OnNewSubQuest(
        const Str& worldName, 
        const Str& subquestName,
        const Str& parentQuestName,
        const int goal
        ) noexcept :
    Message(worldName),
    _subquestName(subquestName),
    _parentQuestName(parentQuestName),
    _goal(goal)
{ /* empty */ }

void OnNewSubQuest::process(MessageProcessor& messageProcessor) const noexcept {
    messageProcessor.onNewSubQuest(
            _worldName, _subquestName, _parentQuestName, _goal);
}


OnNewQuestState::OnNewQuestState(
        const Str& worldName, 
        const Str& questName
        ) noexcept :
    Message(worldName),
    _questName(questName)
{ /* empty */ }

void OnNewQuestState::process(
        MessageProcessor& messageProcessor) const noexcept {
    messageProcessor.onNewQuestState(_worldName, _questName);
}


OnNewQuestStatus::OnNewQuestStatus(
        const Str& worldName, 
        const Str& questName,
        const QuestStatus status
        ) noexcept :
    Message(worldName),
    _questName(questName),
    _status(status)
{ /* empty */ }

void OnNewQuestStatus::process(
        MessageProcessor& messageProcessor) const noexcept {
    messageProcessor.onNewQuestStatus(_worldName, _questName, _status);
}


OnNewQuestPlan::OnNewQuestPlan(
        const mozok::Str& worldName, 
        const mozok::Str& questName,
        const mozok::StrVec& actionList,
        const mozok::Vector<mozok::StrVec>& actionArgsList
        ) noexcept :
    Message(worldName),
    _questName(questName),
    _actionList(actionList),
    _actionArgsList(actionArgsList)
{ /* empty */ }

void OnNewQuestPlan::process(
        mozok::MessageProcessor& messageProcessor) const noexcept {
    messageProcessor.onNewQuestPlan(
            _worldName, _questName, _actionList, _actionArgsList);
}


OnSearchLimitReached::OnSearchLimitReached(
        const Str& worldName, 
        const Str& questName,
        const int searchLimitValue
        ) noexcept :
    Message(worldName),
    _questName(questName),
    _searchLimitValue(searchLimitValue)
{ /* empty */ }

void OnSearchLimitReached::process(
        MessageProcessor& messageProcessor) const noexcept {
    messageProcessor.onSearchLimitReached(
            _worldName, _questName, _searchLimitValue);
}


OnSpaceLimitReached::OnSpaceLimitReached(
        const Str& worldName, 
        const Str& questName,
        const int spaceLimitValue
        ) noexcept :
    Message(worldName),
    _questName(questName),
    _spaceLimitValue(spaceLimitValue)
{ /* empty */ }

void OnSpaceLimitReached::process(
        MessageProcessor& messageProcessor) const noexcept {
    messageProcessor.onSpaceLimitReached(
            _worldName, _questName, _spaceLimitValue);
}

}