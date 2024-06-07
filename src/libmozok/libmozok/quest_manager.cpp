// Copyright 2024 Pavlo Savchuk. Subject to the MIT license.

#include <libmozok/quest_manager.hpp>
#include <libmozok/quest_planner.hpp>

namespace mozok {

const int DEFAULT_SEARCH_LIMIT = 1000;
const int DEFAULT_SPACE_LIMIT = 10000;
const int DEFAULT_OMEGA = 0;

QuestManager::QuestManager(
        const QuestPtr& quest
        ) noexcept :
    _quest(quest),
    _status(MOZOK_QUEST_STATUS_INACTIVE),
    _lastSubstateId(ID(-1)),
    _currentSubstateId(ID(0)),
    _lastActiveGoal(0),
    _searchLimit(DEFAULT_SEARCH_LIMIT),
    _spaceLimit(DEFAULT_SPACE_LIMIT),
    _omega(DEFAULT_OMEGA),
    _parentQuest(nullptr),
    _parentQuestGoal(-1)
{ /* empty */ }

const QuestPtr& QuestManager::getQuest() const noexcept {
    return _quest;
}

QuestStatus QuestManager::getStatus() const noexcept {
    return _status;
}

void QuestManager::activate() noexcept {
    if(_status == MOZOK_QUEST_STATUS_INACTIVE) {
        _status = MOZOK_QUEST_STATUS_UNKNOWN;
    }
}

bool QuestManager::setPlan(QuestPlanPtr lastPlan) noexcept {
    if(lastPlan->givenSubstateId < _lastSubstateId) {
        // Plan is outdated and rejected.
        return false; 
    }
    _lastPlan = lastPlan;
    _status = _lastPlan->status;
    _lastSubstateId = _lastPlan->givenSubstateId;
    _lastActiveGoal = _lastPlan->goalIndx;
    return true;
}

const QuestPlanPtr& QuestManager::getLastPlan() const noexcept {
    return _lastPlan;
}

int QuestManager::getLastActiveGoalIndx() const noexcept {
    return _lastActiveGoal;
}

ID QuestManager::getLastSubstateId() const noexcept {
    return _lastSubstateId;
}

ID QuestManager::getCurrentSubstateId() const noexcept {
    return _currentSubstateId;
}

void QuestManager::increaseCurrentSubstateId() noexcept {
    ++_currentSubstateId;
}

void QuestManager::setQuestStatus(const QuestStatus status, int goal) noexcept {
    _status = status;
    _lastActiveGoal = goal;
}

void QuestManager::setOption( 
        const QuestOption option, 
        const int value
        ) noexcept {
    switch (option) {
    case QUEST_OPTION_SEARCH_LIMIT:
        _searchLimit = value;
        break;
    case QUEST_OPTION_SPACE_LIMIT:
        _spaceLimit = value;
        break;
    case QUEST_OPTION_OMEGA:
        _omega = value;
        break;
    default:
        // skip
        break;
    }
}

void QuestManager::setParentQuest(
        const QuestPtr& parentQuest,
        const int parentGoal
        ) noexcept {
    _parentQuest = parentQuest;
    _parentQuestGoal = parentGoal;
}

const QuestPtr& QuestManager::getParentQuest() const noexcept {
    return _parentQuest;
}

int QuestManager::getParentQuestGoal() const noexcept {
    return _parentQuestGoal;
}

bool QuestManager::performPlanning(
        const Str& worldName,
        const ID substateId,
        const StatePtr& state,
        QuestManagerPtr& questManager,
        MessageProcessor& messageProcessor
        ) noexcept {
    // Skip inactivated quests.
    if(questManager->getStatus() == MOZOK_QUEST_STATUS_INACTIVE)
        return false; 

    // By the Rule 2 quest that is marked as DONE remains DONE.
    if(questManager->getStatus() == MOZOK_QUEST_STATUS_DONE)
        return false; 
    
    // By the Rule 2 quest that is marked as UNREACHABLE remains UNREACHABLE.
    if(questManager->getStatus() == MOZOK_QUEST_STATUS_UNREACHABLE)
        return false; 

    // The status is already known for the given state.
    if(questManager->getLastSubstateId() >= substateId)
        return false;

    // Perform planning.
    const QuestPtr quest = questManager->getQuest();
    QuestPlanner planner(substateId, state, questManager);
    QuestPlanPtr plan = planner.findQuestPlan(
            worldName, 
            messageProcessor, 
            questManager->_searchLimit, 
            questManager->_spaceLimit, 
            questManager->_omega);
    
    const QuestStatus oldStatus = questManager->getStatus();

    // Set a new plan.
    if(questManager->setPlan(plan) == false)
        return false;
    
    // Quest has a new status.
    if(plan->status != oldStatus)
        messageProcessor.onNewQuestStatus(
                worldName, quest->getName(), plan->status);

    StrVec actions;
    Vector<StrVec> actionArgs;
    for(const ActionPtr& action : plan->plan) {
        actions.push_back(action->getName());
        StrVec args;
        for(const ObjectPtr& obj : action->getArguments())
            args.push_back(obj->getName());
        actionArgs.push_back(args);
    }
    messageProcessor.onNewQuestPlan(
        worldName, quest->getName(), actions, actionArgs);

    // Return 'true' because a new plan was found.
    return true;
}


}