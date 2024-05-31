// Copyright 2024 Pavlo Savchuk. Subject to the MIT license.

#include <libmozok/quest.hpp>

namespace mozok {

namespace {
    const ActionPtr nullAction(nullptr);
}


QuestApplicableActionsIterator::~QuestApplicableActionsIterator() noexcept 
    = default;
    
bool QuestApplicableActionsIterator::actionCallback(
        const ActionPtr& /*action*/, const ObjectVec& /*arguments*/) noexcept
{ return true; }


Quest::Quest(
        const Str& name, 
        const ID id,
        const StatementVec& preconditions,
        const GoalVec& goals,
        const ActionVec& actions,
        const ObjectVec& objects,
        const QuestVec& subquests
        ) noexcept :
    _name(name),
    _id(id),
    _preconditions(preconditions),
    _goals(goals),
    _actions(actions),
    _objects(objects),
    _subquests(subquests),
    _actionArgObjects(buildActionArgObjects()),
    _idToAction(buildIdToActionMap()),
    _relevantActions(buildRelevantActions(actions)),
    _relevantObjects(buildRelevantObjects(objects)),
    _relevantRelations(buildRelevantRelations(actions))
{ /* empty */ }

Quest::ActionArgObjects Quest::buildActionArgObjects() const noexcept {
    Vector<Vector<ObjectVec>> result;
    for(const ActionPtr& action : _actions) {
        Vector<ObjectVec> actionArgObjs;
        const ObjectVec& args = action->getArguments();
        bool notEmpty = true;
        for(ObjectVec::size_type i = 0; i < args.size(); ++i) {
            actionArgObjs.push_back({});
            ObjectVec& argObjects = actionArgObjs.at(actionArgObjs.size() - 1);
            for(const ObjectPtr& obj : _objects)
                if(areTypesetsCompatible(obj->getTypeSet(), args[i]->getTypeSet()))
                    argObjects.push_back(obj);
            if(argObjects.size() == 0) {
                notEmpty = false;
                break;
            }
        }
        if(notEmpty) {
            // Action can be applicable.
            result.push_back(actionArgObjs);
        } else {
            // Action is not applicable at all!
            result.push_back({});
        }
    }
    return result;
}

Quest::IdToActionMap Quest::buildIdToActionMap() const noexcept {
    IdToActionMap res;
    for(const ActionPtr& action : _actions)
        res[action->getId()] = action;
    return res;
}

UnorderedSet<ID> Quest::buildRelevantActions(
        const ActionVec& actions) const noexcept {
    UnorderedSet<ID> res;
    for(const ActionPtr& action : actions)
        res.insert(action->getId());
    return res;
}

UnorderedSet<ID> Quest::buildRelevantObjects(
        const ObjectVec& objects) const noexcept {
    UnorderedSet<ID> res;
    for(const ObjectPtr& object : objects)
        res.insert(object->getId());
    return res;
}

UnorderedSet<ID> Quest::buildRelevantRelations(
        const ActionVec& actions) const noexcept {
    UnorderedSet<ID> res;
    for(const ActionPtr& action : actions) {
        for(const StatementPtr& st : action->getPreconditions().getStatements())
            res.insert(st->getRelation()->getId());
        for(const StatementPtr& st : action->getRemList().getStatements())
            res.insert(st->getRelation()->getId());
        for(const StatementPtr& st : action->getAddList().getStatements())
            res.insert(st->getRelation()->getId());
    }
    for(const StatementPtr& st : _preconditions)
        res.insert(st->getRelation()->getId());
    for(const Goal& goal : _goals)
        for(const StatementPtr& st : goal)
            res.insert(st->getRelation()->getId());
    return res;
}

const Str& Quest::getName() const noexcept {
    return _name;
}

ID Quest::getId() const noexcept {
    return _id;
}

const StatementVec& Quest::getPreconditions() const noexcept {
    return _preconditions;
}

const GoalVec& Quest::getGoals() const noexcept {
    return _goals;
}

const ActionVec& Quest::getActions() const noexcept {
    return _actions;
}

const ActionPtr& Quest::getAction(const ID actionId) const noexcept {
    auto it = _idToAction.find(actionId);
    if(it == _idToAction.end())
        return nullAction;
    return it->second;
}

const ObjectVec& Quest::getObjects() const noexcept {
    return _objects;
}

const QuestVec& Quest::getSubquests() const noexcept {
    return _subquests;
}

void Quest::iterateOverApplicableActions(
            const StatePtr& state,
            QuestApplicableActionsIterator& it,
            Vector<StatementVec>& actionPreBuffers
            ) const noexcept {
    for(ActionVec::size_type actionIndx = 0; 
            actionIndx < _actions.size(); ++actionIndx) {
        const ActionPtr& action = _actions[actionIndx];
        if(_actionArgObjects[actionIndx].size() == 0)
            if(action->getArguments().size() > 0)
                continue; // This action is not applicable.
        ObjectVec objects(action->getArguments().size(), ObjectPtr(nullptr));
        ObjectSet objectSet;
        if(findNextObj(state, it, actionPreBuffers, 
                objects, objectSet, actionIndx, 0) == false)
            break; // Stop the search.
    }
}

bool Quest::findNextObj(
        const StatePtr& state,
        QuestApplicableActionsIterator& it,
        Vector<StatementVec>& actionPreBuffers,
        ObjectVec &objects,
        ObjectSet &objectSet,
        ObjectVec::size_type actionIndx,
        ObjectVec::size_type argIndx
        ) const noexcept {
    if(argIndx >= _actionArgObjects[actionIndx].size()) {
        // A new possible substitution has been found.
        const ActionPtr& action = _actions[actionIndx];
        // Objects were selected in such a way that they are suitable by types,
        // but we need to verify if they also satisfy the action preconditions.
        if(action->checkActionPreconditions(
                objects, state, actionPreBuffers[actionIndx]))
            return it.actionCallback(action, objects);
        return true;
    }
    for(const ObjectPtr& obj : _actionArgObjects[actionIndx][argIndx]) {
        if(objectSet.find(obj) != objectSet.end())
            continue;
        objectSet.insert(obj);
        objects[argIndx] = obj;
        if(findNextObj(
                state, it, actionPreBuffers,
                objects, objectSet, actionIndx, 
                argIndx + 1) == false)
            return false;
        objectSet.erase(obj);
    }
    return true;
}

bool Quest::isActionRelevant(const ID actionId) const noexcept {
    return _relevantActions.find(actionId) != _relevantActions.end();
}

bool Quest::isObjectRelevant(const ID objectId) const noexcept {
    return _relevantObjects.find(objectId) != _relevantObjects.end();
}

bool Quest::isRelationRelevant(const ID relationId) const noexcept {
    return _relevantRelations.find(relationId) != _relevantRelations.end();
}

}