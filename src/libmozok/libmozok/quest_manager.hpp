// Copyright 2024 Pavlo Savchuk. Subject to the MIT license.

#pragma once

#include <libmozok/public_types.hpp>
#include <libmozok/message_processor.hpp>
#include <libmozok/private_types.hpp>
#include <libmozok/result.hpp>

#include <libmozok/quest.hpp>
#include <libmozok/quest_plan.hpp>

namespace mozok {

class QuestManager;
using QuestManagerPtr = SharedPtr<QuestManager>;
using QuestManagerVec = Vector<QuestManagerPtr>;


enum QuestOption {
    QUEST_OPTION_SEARCH_LIMIT,
    QUEST_OPTION_SPACE_LIMIT,
    QUEST_OPTION_OMEGA,
    QUEST_OPTION_HEURISTIC
};

enum QuestHeuristic {
    SIMPLE,
    HSP
};


/// @brief Quest settings for planner.
struct QuestSettings {
    /// @brief Maximum number of unique states to visit during search process.
    int searchLimit;

    /// @brief Maximum number of unexplored nodes during the BFS.  
    int spaceLimit;

    /// @brief Special parameter used by the `SIMPLE` heuristics.
    /// (See `quest-format-reference.md` for more details).
    int omega;

    /// @brief Sets the heuristic function used during the A* search.
    QuestHeuristic heuristic;
};


/// @brief Manages the status of a quest.
/// Quest manager rules:
///     1. An active quest cannot become inactive.
///     2. A quest that is marked as DONE remains DONE regardless of 
///        subsequent events.
///     3. It is not possible to revert to previous quest goals.
///     4. A quest that is marked as UNREACHABLE remains UNREACHABLE regardless 
///        of subsequent events.
class QuestManager {
    /// @brief The quest being managed.
    const QuestPtr _quest;

    /// @brief Current status of the quest.
    QuestStatus _status;

    /// @brief The ID of the last substate for which the quest status is known.
    ID _lastSubstateId;

    /// @brief The ID of the current substate of this quest.
    ID _currentSubstateId;

    /// @brief The most recently built plan.
    QuestPlanPtr _lastPlan;

    /// @brief The most recent active goal.
    int _lastActiveGoal;

    /// @brief Various quest settings.
    QuestSettings _settings;

    /// @brief nullptr for a main quest, parent quest for a subquest.
    QuestPtr _parentQuest;

    /// @brief Main quest goal index.
    int _parentQuestGoal;

public:
    QuestManager(const QuestPtr& quest) noexcept;
    const QuestPtr& getQuest() const noexcept;

    /// @brief Activate the inactive quest.
    void activate() noexcept;

    /// @brief Sets a new quest plan.
    /// @param lastPlan The most recently built plan.
    /// @return Returns true if the new plan was accepted. The plan can be 
    ///         rejected if it is outdated and a newer plan for a more recent 
    ///         state is known.
    bool setPlan(QuestPlanPtr lastPlan) noexcept;

    /// @return Returns the most recently built plan.
    const QuestPlanPtr& getLastPlan() const noexcept;

    /// @return Returns the most recent active goal.
    int getLastActiveGoalIndx() const noexcept;

    /// @return Returns the current status of the quest.
    QuestStatus getStatus() const noexcept;

    /// @return Returns the ID of the last substate for which the quest status 
    ///         is known.
    ID getLastSubstateId() const noexcept;

    /// @return Returns the ID of the last substate for which the quest status 
    ///         is known.
    ID getCurrentSubstateId() const noexcept;

    /// @brief Increases the current substate ID. By calling this 
    void increaseCurrentSubstateId() noexcept;

    /// @brief Sets a new quest status. 
    /// Used by the `status` command from a .quest file.
    /// @param status New status.
    /// @param goal New active goal.
    void setQuestStatus(const QuestStatus status, int goal) noexcept;
    
    /// @brief Sets a quest option.
    /// @param option Quest option.
    /// @param value Option value.
    /// @return Returns the status of the operation.
    void setOption( 
            const QuestOption option, 
            const int value
            ) noexcept;

    /// @brief Sets the parent quest.
    /// @param parentQuest Parent quest.
    /// @param parentGoal Parent quest goal that initiates this quest.
    void setParentQuest(
            const QuestPtr& parentQuest,
            const int parentGoal
            ) noexcept;
    
    /// @return nullptr for main quests, parent quest for activated subquests.
    const QuestPtr& getParentQuest() const noexcept;

    /// @return -1 for main quests, parent quest goal for activated subquests.
    int getParentQuestGoal() const noexcept;

    /// @brief Performs planning for the quest.
    /// @param worldName The name of the world where quest lives.
    /// @param substateId Current substate ID of this quest. This state ID must 
    ///         be consistent with the `state` value.
    /// @param state Current state of the world. This state must be consistent 
    ///         with the `substateId` value.
    /// @param questManager The manager of the quest.
    /// @param messageProcessor A message processor for handling messages.
    /// @return Returns true if a new plan was found.
    static bool performPlanning(
            const Str& worldName,
            const ID substateId,
            const StatePtr& state,
            QuestManagerPtr& questManager,
            MessageProcessor& messageProcessor
            ) noexcept;

};

}
