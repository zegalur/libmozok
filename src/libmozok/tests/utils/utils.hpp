// Copyright 2024-2025 Pavlo Savchuk. Subject to the MIT license.

#pragma once

#include <memory>
#include <libmozok/mozok.hpp>

namespace mozok {
class DebugMessageProcessor : public MessageProcessor {
public:
    void onActionError(
            const Str&, 
            const Str& actionName,
            const StrVec& actionArguments,
            const Result& errorResult,
            const mozok::ActionError actionError,
            const int data
            ) noexcept override;
    void onNewMainQuest(
            const Str&, 
            const Str& questName
            ) noexcept override;
    void onNewSubQuest(
            const Str& , 
            const Str& subquestName,
            const Str& parentQuestName,
            const int goal
            ) noexcept override;
    void onNewQuestStatus(
            const Str&, 
            const Str& questName,
            const QuestStatus questStatus
            ) noexcept override;
    void onNewQuestGoal(
            const Str&,
            const Str& questName,
            const int newGoal,
            const int oldGoal
            ) noexcept override;
    void onNewQuestPlan(
            const Str&, 
            const Str& questName,
            const StrVec& actionList,
            const Vector<StrVec>& actionArgsList
            ) noexcept override;
    void onSearchLimitReached(
            const mozok::Str&,
            const mozok::Str& questName,
            const int searchLimitValue
            ) noexcept override;
    void onSpaceLimitReached(
            const mozok::Str&,
            const mozok::Str& questName,
            const int spaceLimitValue
            ) noexcept override;
};
}


/// @brief Creates a server and adds single project file.
/// @param serverName Server name.
/// @param worldName World name.
/// @param fileName Project's `.quest` file path.
/// @param out The status of the operation will be saved into this variable.
/// @return Returns a newly created quest server.
std::unique_ptr<mozok::Server> createServerFromFile(
        const mozok::Str& serverName,
        const mozok::Str& worldName,
        const mozok::Str& fileName,
        mozok::Result& out);
