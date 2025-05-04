// Copyright 2024-2025 Pavlo Savchuk. Subject to the MIT license.

#include <libmozok/action.hpp>
#include <libmozok/public_types.hpp>
#include <libmozok/message_processor.hpp>
#include <libmozok/world.hpp>
#include <libmozok/error_utils.hpp>
#include <libmozok/project.hpp>

#include <sstream>

namespace mozok {

namespace {
    const TypePtr nullType(nullptr);
    const ObjectPtr nullObject(nullptr);
    const RelationPtr nullRelation(nullptr);
    const RelationListPtr nullRelationList(nullptr);
    const ActionPtr nullAction(nullptr);
    QuestManagerPtr nullQuest(nullptr);
}

World::World(const Str& serverName, const Str& worldName) noexcept :
        _serverName(serverName), 
        _worldName(worldName), 
        _serverWorldName(_serverName + ":" + _worldName),
        _state(makeShared<State>(StatementVec())),
        _stateId(ID(0))
{ /* empty */ }

const Str& World::getServerName() const noexcept {
    return _serverName;
}

const Str& World::getWorldName() const noexcept {
    return _worldName;
}

const Str& World::getServerWorldName() const noexcept {
    return _serverWorldName;
}

Str World::generateSaveFile() noexcept {
    const StatePtr stateCopy = _state->duplicate();

    std::stringstream res;
    res << "# Save file for '" << _serverWorldName << "'" << std::endl;
    res << "version 1 0" << std::endl;
    res << "project " << _worldName << std::endl;
    res << std::endl;
    res << "action Load:" << std::endl;
    for(const QuestManagerVec* quests : {&_mainQuests, &_subquests})
        for(QuestManagerVec::size_type rIndx=0; rIndx<(*quests).size(); ++rIndx) {
            // To ensure that any parent quest is always listed before its 
            // subquests, we output the quest status commands in the reverse 
            // order of their definitions. This works, because `subquests:` 
            // can only reference subquests that have been previously defined.
            const QuestManagerPtr& quest = (*quests)[(*quests).size()-rIndx-1];
            res << "    status " << quest->getQuest()->getName() << " ";
            if(quest->getStatus() == MOZOK_QUEST_STATUS_INACTIVE)
                res << "INACTIVE ";
            else if(quest->getStatus() == MOZOK_QUEST_STATUS_UNREACHABLE)
                res << "UNREACHABLE ";
            else if(quest->getStatus() == MOZOK_QUEST_STATUS_DONE)
                res << "DONE " << quest->getLastActiveGoalIndx();
            else
                res << "ACTIVE " << quest->getLastActiveGoalIndx();
            if(quest->getStatus() != MOZOK_QUEST_STATUS_INACTIVE
                    && quest->getParentQuest())
                res << " PARENT " << quest->getParentQuest()->getName()
                    << " " << quest->getParentQuestGoal();
            res << std::endl;
        }
    res << "    pre # none" << std::endl;
    res << "    rem # none" << std::endl;
    res << "    add # Current State:" << std::endl;
    // Add the offset to lineup the statements for readability.
    res << "        ";
    for(const StatementPtr& st : stateCopy->getStatementSet()) {
        res << st->getRelation()->getName() << "(";
        for(ObjectVec::size_type i=0; i<st->getArguments().size(); ++i) {
            res << st->getArguments()[i]->getName();
            if(i != st->getArguments().size()-1)
                res << ", ";
        }
        res << ")" << std::endl;
        // Add the offset to lineup the statements for readability.
        res << "        ";
    }
    return res.str();
}


// =============================== PROJECT ================================== //

Result World::addProject(
        const Str& projectFileName, const Str& projectSrc) noexcept {
    return addFromProjectSRC(this, projectFileName, projectSrc);
}


// ================================= TYPE =================================== //

const TypePtr& World::getType(const Str& typeName) const noexcept {
    if(hasType(typeName))
        return _types[_typeNameToId.find(typeName)->second];
    return nullType;
}

Result World::constructFullType(const StrVec& types, TypeSet& out) const noexcept {
    for(const Str &supertypeName : types) {
        if(hasType(supertypeName) == false)
            return errorUndefinedType(this->getServerWorldName(), supertypeName);
        TypePtr type = getType(supertypeName);
        out.insert(type);
        out.insert(type->getSupertypes().begin(), 
                   type->getSupertypes().end());
    }
    return Result::OK();
}

Result World::constructTypeVec(const StrVec& types, TypeVec& out) const noexcept {
    for(const Str &supertypeName : types) {
        if(hasType(supertypeName) == false)
            return errorUndefinedType(this->getServerWorldName(), supertypeName);
        TypePtr type = getType(supertypeName);
        out.push_back(type);
    }
    return Result::OK();
}

Result World::addType(
        const Str& typeName,
        const StrVec& supertypeNames
        ) noexcept {
    const Result definitionError = errorTypeCantDefine(
            getServerWorldName(), typeName);
    if(hasType(typeName))
        return (errorTypeAlreadyExists(this->getServerWorldName(), typeName)
                <<= definitionError);
        
    TypeSet supertypes;
    Result res = constructFullType(supertypeNames, supertypes);
    if(res.isError())
        return res <<= definitionError;

    // Create and store the type.
    const ID newTypeId = (ID)_types.size();
    _typeNameToId[typeName] = newTypeId;
    TypePtr newType = makeShared<Type>(typeName, newTypeId, supertypes);
    _types.push_back(newType);

    return Result::OK();
}

bool World::hasType(const Str& typeName) const noexcept {
    return _typeNameToId.find(typeName) != _typeNameToId.end();
}


// ================================ OBJECT ================================== //

const ObjectPtr& World::getObject(const Str& objectName) const noexcept {
    if(hasObject(objectName)) 
        return _objects[_objectNameToId.find(objectName)->second];
    return nullObject;
}

Result World::addObject(
        const Str& objectName,
        const StrVec& typeNames
        ) noexcept {
    const Result definitionError = errorObjectCantDefine(
            getServerWorldName(), objectName);
    if(hasObject(objectName))
        return (errorObjectAlreadyExists(this->getServerWorldName(), objectName)
                <<= definitionError);
        
    TypeSet fullType;
    Result res = constructFullType(typeNames, fullType);
    if(res.isError())
        return res <<= definitionError;

    // Create and store the object.
    const ID newObjectId = (ID)_objects.size();
    _objectNameToId[objectName] = newObjectId;
    ObjectPtr newObject = makeShared<Object>(objectName, newObjectId, fullType);
    _objects.push_back(newObject);

    return Result::OK();
}

bool World::hasObject(const Str& objectName) const noexcept {
    return _objectNameToId.find(objectName) != _objectNameToId.end();
}

StrVec World::getObjects() const noexcept {
    StrVec res;
    for(const auto& obj : _objects)
        res.push_back(obj->getName());
    return res;
}

StrVec World::getObjectType(const Str& objectName) const noexcept {
    if(hasObject(objectName) == false)
        return {};
    const auto it = _objectNameToId.find(objectName);
    const auto typeSet = _objects[it->second]->getTypeSet();
    StrVec res;
    for(const auto& t : typeSet)
        res.push_back(t->getName());
    return res;
}

Result World::constructArguments(
        const Vector<StrVec> &arguments, ObjectVec& out) const noexcept {
    ID id = -1;
    for(Vector<StrVec>::size_type i = 0; i < arguments.size(); ++i, --id) {
        const auto& arg = arguments[i];
        const Str& argName = arg.front();
        if(hasObject(argName))
            return errorObjectAlreadyExists(getServerWorldName(), argName);

        TypeSet argFullType;
        const StrVec argTypes(arg.begin() + 1, arg.end());
        Result res = constructFullType(argTypes, argFullType);
        if(res.isError())
            return res;

        const ObjectPtr argObj = makeShared<Object>(argName, id, argFullType);
        out.push_back(argObj);
    }
    return Result::OK();
}


// =============================== RELATION ================================= //

const RelationPtr& World::getRelation(const Str& relationName) const noexcept {
    if(hasRelation(relationName)) 
        return _relations[_relationNameToId.find(relationName)->second];
    return nullRelation;
}

Result World::addRelation(
        const Str& relationName,
        const StrVec& argumentTypeNames
        ) noexcept {
    const Result definitionError = errorRelationCantDefine(
            getServerWorldName(), relationName);
    if(hasRelation(relationName))
        return (errorRelAlreadyExists(this->getServerWorldName(), relationName)
                <<= definitionError);
    
    TypeVec argTypes;
    Result res = constructTypeVec(argumentTypeNames, argTypes);
    if(res.isError())
        return res <<= definitionError;

    // Create and store the relation.
    const ID newRelationId = (ID)_relations.size();
    _relationNameToId[relationName] = newRelationId;
    RelationPtr newRelation = makeShared<Relation>(
            relationName, newRelationId, argTypes);
    _relations.push_back(newRelation);

    return Result::OK();
}

bool World::hasRelation(const Str& relationName) const noexcept {
    return _relationNameToId.find(relationName) != _relationNameToId.end();
}

Result World::constructStatements(
        const Vector<StrVec> &list, 
        const ObjectVec& localObjects,
        StatementVec& out
        ) const noexcept {
    UnorderedMap<Str, ObjectPtr> locObjectsMap;
    for(const auto& obj : localObjects)
        locObjectsMap[obj->getName()] = obj;

    int commandIndx = 0;
    for(const auto& command : list) {
        const Str& commandName = command.front();
        commandIndx++;

        const bool isRelation = hasRelation(commandName);
        const bool isRList = hasRelationList(commandName);
        if(!isRelation && !isRList)
            return errorUndefinedRel(getServerWorldName(), commandName);
        
        ObjectVec statementArgs;
        for(StrVec::size_type i = 1 /* skip the command name */; 
                i < command.size(); 
                ++i) {
            const Str& argName = command[i];
            ObjectPtr argObj;
            if(locObjectsMap.find(argName) != locObjectsMap.end())
                argObj = locObjectsMap[argName];
            else if(hasObject(argName))
                argObj = getObject(argName);
            else {
                return (
                    errorUndefinedObject(getServerWorldName(), argName)
                        <<= errorWorldOtherError(getServerWorldName(), 
                            Str("Error in '") + commandName 
                            + Str("(...)' (") 
                            + Str(std::to_string(commandIndx).c_str()) 
                            + Str("-th statement, "
                            + Str(std::to_string(i).c_str())
                            + "-th argument) (see previous error).")));
            }
            statementArgs.push_back(argObj);
        }

        if(isRelation) {
            const RelationPtr relation = getRelation(commandName);
            Result res = relation->checkArgumentsCompatibility(statementArgs);
            if(res.isError())
                return (res <<= errorWorldOtherError(getServerWorldName(), 
                        "Incompatible arguments (see previous error)."));
            out.push_back(makeShared<Statement>(
                    getRelation(commandName), statementArgs));
        } else {
            const RelationListPtr rlist = getRelationList(commandName);
            Result res = rlist->checkArgumentsCompatibility(statementArgs);
            if(res.isError())
                return (res <<= errorWorldOtherError(getServerWorldName(), 
                        "Incompatible arguments (see previous error)."));
            const StatementVec substitution = rlist->substitute(statementArgs);
            for(const StatementPtr& statement : substitution)
                out.push_back(statement);
        }
    }

    return Result::OK();
}


// ============================ RELATION LIST =============================== //

const RelationListPtr& World::getRelationList(const Str& rlistName) const noexcept {
    if(hasRelationList(rlistName)) 
        return _relationLists[_relationListNameToId.find(rlistName)->second];
    return nullRelationList;
}

Result World::addRelationList(
        const Str& relationListName,
        const Vector<StrVec> &arguments,
        const Vector<StrVec> &list
        ) noexcept {
    const Result definitionError = errorRListCantDefine(
            getServerWorldName(), relationListName);
    if(hasRelationList(relationListName))
        return (errorRListAlreadyExists(getServerWorldName(), relationListName)
                <<= definitionError);
    
    Result res;

    ObjectVec argObjects;
    res <<= constructArguments(arguments, argObjects);
    if(res.isError())
        return res <<= definitionError;
    
    StatementVec statements;
    res <<= constructStatements(list, argObjects, statements);
    if(res.isError())
        return res <<= definitionError;
    
    // Create and store the relation list.
    const ID newRelationListId = (ID)_relationLists.size();
    _relationListNameToId[relationListName] = newRelationListId;
    RelationListPtr newRelationList = makeShared<RelationList>(
            relationListName, newRelationListId, argObjects, statements);
    _relationLists.push_back(newRelationList);

    return Result::OK();
}

bool World::hasRelationList(const Str& relationListName) const noexcept {
    return _relationListNameToId.find(relationListName) != _relationListNameToId.end();
}


// ================================ ACTION ================================== //

Result World::addActionGroup(const Str& actionGroupName) noexcept {
    if(hasActionGroup(actionGroupName) == true)
        return errorActionGroupAlreadyExists(
                getServerWorldName(), actionGroupName);
    _actionGroups[actionGroupName] = {};
    return Result::OK();
}

bool World::hasActionGroup(const Str& actionGroupName) const noexcept {
    return _actionGroups.find(actionGroupName) != _actionGroups.end();
}

const ActionPtr& World::getAction(const Str& actionName) const noexcept {
    if(hasAction(actionName)) 
        return _actions[_actionNameToId.find(actionName)->second];
    return nullAction;
}

Result World::addAction(
        const Str& actionName,
        const StrVec& actionGroups,
        const bool isNotApplicable,
        const Vector<StrVec> &arguments,
        const Vector<StrVec> &preList,
        const Vector<StrVec> &remList,
        const Vector<StrVec> &addList
        ) noexcept {
    const Result definitionError = errorActionCantDefine(
            getServerWorldName(), actionName);
    if(hasAction(actionName))
        return (errorActionAlreadyExists(getServerWorldName(), actionName)
                <<= definitionError);
    for(const auto& groupName : actionGroups)
        if(hasActionGroup(groupName) == false)
            return (errorUndefinedActionGroup(getServerWorldName(), groupName))
                    <<= definitionError;
    
    Result res;

    ObjectVec argObjects;
    res <<= constructArguments(arguments, argObjects);
    if(res.isError())
        return res <<= definitionError;
    
    StatementVec pre;
    StatementVec rem;
    StatementVec add;

    res <<= constructStatements(preList, argObjects, pre);
    if(res.isError())
        return res <<= errorActionPreError() <<= definitionError;
    
    res <<= constructStatements(remList, argObjects, rem);
    if(res.isError())
        return res <<= errorActionRemError() <<= definitionError;
    
    res <<= constructStatements(addList, argObjects, add);
    if(res.isError())
        return res <<= errorActionAddError() <<= definitionError;
    
    // Create and store the action.
    const ID newActionId = (ID)_actions.size();
    _actionNameToId[actionName] = newActionId;
    ActionPtr newAction = makeShared<Action>(
            actionName, newActionId, isNotApplicable, argObjects, pre, rem, add);
    _actions.push_back(newAction);

    // Add action to the action groups.
    for(const auto& groupName : actionGroups)
        _actionGroups[groupName].push_back(newAction);

    return Result::OK();
}

bool World::hasAction(const Str& actionName) const noexcept {
    return _actionNameToId.find(actionName) != _actionNameToId.end();
}

bool World::isActionNotApplicable(const Str& actionName) const noexcept {
    if(hasAction(actionName) == false)
        return true;
    return getAction(actionName)->isNotApplicable();
}

Result World::applyAction(
        const Str& actionName,
        const StrVec& actionArguments,
        MessageProcessor& messageProcessor,
        ActionError& errorOutput
        ) noexcept {
    if(hasAction(actionName) == false) {
        errorOutput = MOZOK_AE_UNDEFINED_ACTION;
        return errorUndefinedAction(_serverWorldName, actionName);
    }
    const ActionPtr action = getAction(actionName);

    if(action->isNotApplicable()) {
        errorOutput = MOZOK_AE_NA_ACTION;
        return errorCantApplyNAAction(_serverWorldName, actionName);
    }

    ObjectVec objects;
    for(const Str& objName : actionArguments)
        if(hasObject(objName))
            objects.push_back(getObject(objName));
        else {
            errorOutput = MOZOK_AE_UNDEFINED_OBJECT;
            return errorUndefinedObject(_serverWorldName, objName);
        }

    Result res = action->applyAction(objects, _state, errorOutput);
    if(res.isError())
        return res;
    
    // Action was successfully applied, state has been changed.
    ++_stateId;

    // Apply status change operations.
    const auto& it = _actionStatusChangeCommands.find(action->getId());
    if(it != _actionStatusChangeCommands.end())
        for(QuestStatusChangeCommand& cmd : it->second) {
            const QuestStatus prevStatus = cmd.quest->getStatus();
            // Ensure that `onNewMainQuest` and `onNewSubQuest` is always before
            // the `onNewQuestStatus` message.
            if(cmd.parentQuest) {
                // This quest is a subquest
                if(prevStatus == MOZOK_QUEST_STATUS_INACTIVE)
                    if(cmd.status != MOZOK_QUEST_STATUS_INACTIVE) {
                        cmd.quest->setParentQuest(
                                cmd.parentQuest->getQuest(), cmd.parentGoal);
                        // add `onNewSubQuest` message
                        messageProcessor.onNewSubQuest(
                                _worldName, cmd.quest->getQuest()->getName(),
                                cmd.parentQuest->getQuest()->getName(),
                                cmd.parentGoal);
                    }
            } else {
                // This quest is a main quest
                if(prevStatus == MOZOK_QUEST_STATUS_INACTIVE)
                    if(cmd.status != MOZOK_QUEST_STATUS_INACTIVE) {
                        // add `onNewMainQuest` message
                        messageProcessor.onNewMainQuest(
                            _worldName, cmd.quest->getQuest()->getName());
                    }
            }
            // Status change command increases the substate id
            cmd.quest->increaseCurrentSubstateId();
            int oldGoal = cmd.quest->getLastActiveGoalIndx();
            cmd.quest->setQuestStatus(cmd.status, cmd.goal);
            // Do not send `onNewQuestStatus` if the quest was `INACTIVE` 
            // and remains `INACTIVE`.
            if((prevStatus == MOZOK_QUEST_STATUS_INACTIVE 
                    && cmd.status == MOZOK_QUEST_STATUS_INACTIVE) == false)
                messageProcessor.onNewQuestStatus(
                        _worldName, cmd.quest->getQuest()->getName(), cmd.status);
            // Trigger `onNewQuestGoal` when goal changes or when
            // quest was INACTIVE and now it's not INACTIVE or UNKNOWN.
            if(oldGoal != cmd.goal 
                    || (prevStatus == MOZOK_QUEST_STATUS_INACTIVE 
                            && cmd.status != MOZOK_QUEST_STATUS_INACTIVE
                            && cmd.status != MOZOK_QUEST_STATUS_UNKNOWN))
                messageProcessor.onNewQuestGoal(
                        _worldName, cmd.quest->getQuest()->getName(), 
                        cmd.goal, oldGoal);
        }

    // Change the substate IDs of relevant quests
    for(QuestManagerVec* qms : {&_mainQuests, &_subquests})
        for(QuestManagerPtr& qm : (*qms)) {
            if(qm->getStatus() == MOZOK_QUEST_STATUS_INACTIVE)
                continue;
            if(qm->getStatus() == MOZOK_QUEST_STATUS_DONE)
                continue;
            if(qm->getStatus() == MOZOK_QUEST_STATUS_UNREACHABLE)
                continue;
            if(action->isGlobal()) {
                qm->increaseCurrentSubstateId();
                messageProcessor.onNewQuestState(
                        _worldName, qm->getQuest()->getName());
            } else {
                // It doesn't matter if the action is relevant. If a relevant 
                // action is applied without any relevant arguments, such an 
                // action couldn't change a quest status in any way.
                // That's why instead of:
                //   const ID actionId = action->getId();
                //   bool relevant = qm->getQuest()->isActionRelevant(actionId);
                // it is better to have:
                bool relevant = false;
                for(const ObjectPtr& obj : objects) {
                    if(relevant)
                        break;
                    if(qm->getQuest()->isObjectRelevant(obj->getId()))
                        relevant = true;
                }
                if(relevant) {
                    qm->increaseCurrentSubstateId();
                    messageProcessor.onNewQuestState(
                        _worldName, qm->getQuest()->getName());
                }
            }
        }
    
    activateInactiveMainQuests(messageProcessor);
    
    return Result::OK();
}

Result World::checkAction(
        const bool doNotCheckPreconditions,
        const Str& actionName,
        const StrVec& actionArguments
        ) const noexcept {
    if(hasAction(actionName) == false)
        return errorUndefinedAction(_serverWorldName, actionName);
    
    ObjectVec objects;
    for(const Str& objName : actionArguments)
        if(hasObject(objName))
            objects.push_back(getObject(objName));
        else
            return errorUndefinedObject(_serverWorldName, objName);

    ActionError actionError;
    return getAction(actionName)->evaluateActionApplicability(
            doNotCheckPreconditions, objects, _state, actionError);
}


Result World::addActionQuestStatusChange(
        const Str& actionName,
        const Str& questName, 
        const QuestStatus status,
        int goal,
        const Str& parentQuestName,
        int parentQuestGoal
        ) noexcept {
    if(hasAction(actionName) == false)
        return errorUndefinedAction(_serverWorldName, actionName);
    if(hasMainQuest(questName) == false && hasSubquest(questName) == false)
        return errorUndefinedQuest(_serverWorldName, questName);
    
    // If this is a subquest status command.
    if(parentQuestName.length() > 0) {
        if(hasMainQuest(parentQuestName) == false 
                && hasSubquest(parentQuestName) == false) {
            // Undefined parent quest
            return errorUndefinedQuest(_serverWorldName, parentQuestName);
        }
        if(hasSubquest(questName) == false) {
            // This quest must be a subquest.
            return errorUndefinedSubQuest(_serverWorldName, questName);
        }
    }

    const ID actionId = getAction(actionName)->getId();

    // Get quest manager
    QuestManagerPtr& quest = (hasMainQuest(questName) 
            ? getMainQuest(questName) 
            : getSubquest(questName));
    
    // Get parent quest manager
    QuestManagerPtr& parentQuest = (parentQuestName.length() == 0
            ? nullQuest
            : (hasMainQuest(parentQuestName) 
                ? getMainQuest(parentQuestName) 
                : getSubquest(parentQuestName)));

    // Check if goal index is correct.
    if(goal < 0 || GoalVec::size_type(goal) >= quest->getQuest()->getGoals().size())
        return errorActionSetStatusGoalError(
                _serverWorldName, actionName, questName, goal);
    
    // Check if parent quest goal is correct.
    const GoalVec::size_type pGoal = GoalVec::size_type(parentQuestGoal);
    if(parentQuest)
        if(parentQuestGoal < 0 || pGoal >= parentQuest->getQuest()->getGoals().size())
            return errorActionSetStatusParentGoalError(
                    _serverWorldName, actionName, parentQuestName, parentQuestGoal);
    
    _actionStatusChangeCommands[actionId].push_back(
            {quest, status, goal, parentQuest, parentQuestGoal});
    
    return Result::OK();
}

StrVec World::getActions() const noexcept {
    StrVec res;
    for(const auto& a : _actions)
        res.push_back(a->getName());
    return res;
}

Vector<StrVec> World::getActionType(
        const mozok::Str& actionName
        ) const noexcept {
    if(hasAction(actionName) == false)
        return {};
    Vector<StrVec> res;
    const auto &a = _actions[_actionNameToId.find(actionName)->second];
    for(const auto& arg : a->getArguments()) {
        StrVec avec;
        avec.push_back(arg->getName());
        for(const auto& t : arg->getTypeSet())
            avec.push_back(t->getName());
        res.push_back(avec);
    }
    return res;
}


// ================================= QUEST ================================== //

QuestManagerPtr& World::getMainQuest(const Str& questName) noexcept {
    if(hasMainQuest(questName)) 
        return _mainQuests[_mainQuestNameToId.find(questName)->second];
    return nullQuest;
}

QuestManagerPtr& World::getSubquest(const Str& questName) noexcept {
    if(hasSubquest(questName)) 
        return _subquests[_subquestNameToId.find(questName)->second];
    return nullQuest;
}

const QuestManagerPtr& World::getMainQuest(const Str& questName) const noexcept {
    if(hasMainQuest(questName)) 
        return _mainQuests[_mainQuestNameToId.find(questName)->second];
    return nullQuest;
}

const QuestManagerPtr& World::getSubquest(const Str& questName) const noexcept {
    if(hasSubquest(questName)) 
        return _subquests[_subquestNameToId.find(questName)->second];
    return nullQuest;
}

Result World::addQuest(
        const Str& questName,
        const bool isMainQuest,
        const Vector<StrVec> &preconditions,
        const Vector<Vector<StrVec>> &goals,
        const StrVec& questActionNames,
        const StrVec& questObjectNames,
        const StrVec& questSubquestNames,
        const bool useActionTree
        ) noexcept {
    const Result definitionError = errorQuestCantDefine(
            getServerWorldName(), questName);
        
    if(hasMainQuest(questName) || hasSubquest(questName))
        return (errorQuestAlreadyExists(getServerWorldName(), questName)
                <<= definitionError);
    
    Result res;

    StatementVec pre;
    res <<= constructStatements(preconditions, {}, pre);
    if(res.isError())
        return res <<= errorQuestPreconditionsError() <<= definitionError;
    
    GoalVec goalVec;
    for(Vector<StrVec>::size_type i=0; i<goals.size(); ++i) {
        const Vector<StrVec>& goalList = goals[i];
        Goal goal;
        res <<= constructStatements(goalList, {}, goal);
        if(res.isError())
            return res <<= errorQuestGoalError(int(i)) <<= definitionError;
        goalVec.push_back(goal);
    }

    ActionVec actions;
    ActionSet addedActions;
    bool hasActionsError = false;
    for(const Str& actionName : questActionNames) {
        if(actionName.length()>0 && actionName[0]>='A' && actionName[0]<='Z') {
            // Action name.
            if(hasAction(actionName) == true) {
                const auto& action = getAction(actionName);
                if(addedActions.count(action) == 0) {
                    actions.push_back(action);
                    addedActions.insert(action);
                }
            } else {
                res <<= errorUndefinedAction(getServerWorldName(), actionName);
                hasActionsError = true;
            }
        } else {
            // Action group name.
            if(hasActionGroup(actionName) == false) {
                res <<= errorUndefinedActionGroup(getServerWorldName(), actionName);
                hasActionsError = true;
            } else {
                const auto& agroup = _actionGroups[actionName];
                for(const auto& action : agroup)
                    if(addedActions.count(action) == 0) {
                        actions.push_back(action);
                        addedActions.insert(action);
                    }
            }
        }
    }

    // Check actions locality. Only local action can be listed as quest action.
    for(ActionVec::size_type i = 0; i < actions.size(); ++i) {
        const ActionPtr& action = actions[i];
        if(action->isGlobal())
            res <<= errorQuestActionIsGlobal(questName, action->getName());
    }

    if(hasActionsError) res <<= errorQuestActionsError();

    bool hasObjectsError = false;
    ObjectVec objects;
    ObjectSet added;
    for(const Str& objectName : questObjectNames) {
        if(objectName.length()>0 && objectName[0]>='A' && objectName[0]<='Z') {
            // A type name.
            if(hasType(objectName) == false) {
                res <<= errorUndefinedType(getServerWorldName(), objectName);
                hasObjectsError = true;
            } else {
                for(const auto& obj : _objects)
                    if(areTypesetsCompatible(
                            obj->getTypeSet(), {getType(objectName)}) == true)
                        if(added.count(obj) == 0) {
                            objects.push_back(obj);
                            added.insert(obj);
                        }
            }
        } else {
            // An object name.
            if(hasObject(objectName) == true) {
                const auto& obj = getObject(objectName);
                if(added.count(obj) == 0) {
                    objects.push_back(obj);
                    added.insert(obj);
                }
            } else {
                res <<= errorUndefinedObject(getServerWorldName(), objectName);
                hasObjectsError = true;
            }
        }
    }
    if(hasObjectsError) res <<= errorQuestObjectsError();
    
    QuestVec subquests;
    bool hasSubquestsError = false;
    for(const Str& subquestName : questSubquestNames) {
        if(hasSubquest(subquestName) == true)
            subquests.push_back(getSubquest(subquestName)->getQuest());
        else {
            res <<= errorUndefinedQuest(getServerWorldName(), subquestName);
            hasSubquestsError = true;
        }
    }
    if(hasSubquestsError) res <<= errorQuestSubquestsError();

    if(res.isError())
        return res <<= definitionError;

    // Create and store the quest.
    QuestManagerVec& _quests = (isMainQuest ? _mainQuests : _subquests);
    auto& _questNameToId = (isMainQuest ? _mainQuestNameToId : _subquestNameToId);
    const ID newQuestId = (ID)_quests.size();
    _questNameToId[questName] = newQuestId;
    QuestPtr newQuest = makeShared<Quest>(
            questName, newQuestId, pre, goalVec, 
            actions, objects, subquests, useActionTree);
    QuestManagerPtr newQuestManager = makeShared<QuestManager>(newQuest);
    _quests.push_back(newQuestManager);

    return Result::OK();
}

bool World::hasMainQuest(const Str& questName) const noexcept {
    return _mainQuestNameToId.find(questName) != _mainQuestNameToId.end();
}

bool World::hasSubquest(const Str& questName) const noexcept {
    return _subquestNameToId.find(questName) != _subquestNameToId.end();
}

void World::activateInactiveMainQuests(
        MessageProcessor& messageProcessor) noexcept {
    // Find inactive main quests that now require activation.
    for(QuestManagerPtr& mainQuest : _mainQuests)
        if(mainQuest->getStatus() == MOZOK_QUEST_STATUS_INACTIVE)
            if(_state->hasSubstate(mainQuest->getQuest()->getPreconditions())) {
                mainQuest->activate();
                messageProcessor.onNewMainQuest(
                        _worldName, mainQuest->getQuest()->getName());
            }
}

QuestStatus World::getQuestStatus(const Str& questName) const noexcept {
    if(hasMainQuest(questName))
        return getMainQuest(questName)->getStatus();
    if(hasSubquest(questName))
        return getSubquest(questName)->getStatus();
    return MOZOK_QUEST_STATUS_INACTIVE;
}

Result World::setQuestOption(
        const Str& questName, 
        const QuestOption option,
        const int value
        ) noexcept {
    if(hasMainQuest(questName) == false && hasSubquest(questName) == false)
        return errorUndefinedQuest(_serverWorldName, questName);
    if(hasMainQuest(questName))
        getMainQuest(questName)->setOption(option, value);
    if(hasSubquest(questName))
        getSubquest(questName)->setOption(option, value);
    return Result::OK();
}


// ================================ PLANNING ================================ //

void World::performPlanning(
        MessageProcessor& messageProcessor
        ) noexcept {
    for(QuestManagerVec* questSet : {&_mainQuests, &_subquests})
        for(QuestManagerPtr& questManager : (*questSet)) {
            if(questManager->getStatus() == MOZOK_QUEST_STATUS_INACTIVE)
                continue;
            if(questManager->getStatus() == MOZOK_QUEST_STATUS_DONE)
                continue;
            if(questManager->getLastSubstateId() 
                    == questManager->getCurrentSubstateId())
                continue;
            performQuestPlanning(questManager, messageProcessor);
        }
}

void World::performQuestPlanning(
        QuestManagerPtr& questManager,
        MessageProcessor& messageProcessor
        ) noexcept {
    // Create a duplicate substate with relevant statements only.
    StatePtr planningState = _state->duplicate(*questManager->getQuest());
    const ID planningSubstateID = questManager->getCurrentSubstateId();

    bool newPlan = QuestManager::performPlanning(
            _worldName, planningSubstateID, planningState, 
            questManager, messageProcessor);

    if(newPlan)
        findNewSubquest(questManager, messageProcessor);
}

void World::findNewSubquest(
        QuestManagerPtr& questManager,
        MessageProcessor& messageProcessor
        ) noexcept {
    const QuestPlanPtr& plan = questManager->getLastPlan();
    const QuestPtr& quest = questManager->getQuest();
    if(quest->getSubquests().size() == 0)
        return;
    if(plan->plan.size() == 0)
        return;
    const ActionPtr planAction = plan->plan.front();
    const ActionPtr action = quest->getAction(planAction->getId());

    // Only N/A actions can trigger subquests.
    if(action->isNotApplicable() == false)
        return;

    StatePtr post = plan->givenState->duplicate();
    ActionError actionError = MOZOK_AE_NO_ERROR;
    action->applyAction(planAction->getArguments(), post, actionError);
    for(const QuestPtr& subquest : plan->quest->getSubquests()) {
        QuestManagerPtr subquestManager = getSubquest(subquest->getName());
        if(subquestManager->getStatus() != MOZOK_QUEST_STATUS_INACTIVE)
            continue;
        if(!plan->givenState->hasSubstate(subquest->getPreconditions()))
            continue;
        //int goalIndx = 0;
        for(const Goal& goal : subquest->getGoals()) {
            if(post->hasSubstate(goal)) {
                // New subquest has been found.
                // Activate the subquest.
                subquestManager->setParentQuest(quest, plan->goalIndx);
                subquestManager->activate();
                messageProcessor.onNewSubQuest(
                    _worldName, 
                    subquest->getName(), 
                    quest->getName(),
                    plan->goalIndx);
                // Perform planning for the new subquest.
                performQuestPlanning(subquestManager, messageProcessor);
                break;
            }
            //++goalIndx;
        }
    }
}


}
