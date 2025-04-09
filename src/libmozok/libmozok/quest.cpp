// Copyright 2024 Pavlo Savchuk. Subject to the MIT license.

#include <libmozok/public_types.hpp>
#include <libmozok/statement.hpp>
#include <libmozok/quest.hpp>

namespace mozok {

namespace {

const ActionPtr nullAction(nullptr);
const StatePtr nullState(nullptr);

/// @brief Fills the `PossibleActionVec` vector with data provided by an 
///        `iterateOverApplicableActions` call.
class PossibleActionsBuilder : public QuestApplicableActionsIterator {
    Quest::PossibleActionVec& _possibleActions;
public:
    PossibleActionsBuilder(
            Quest::PossibleActionVec& possibleActions)
        : _possibleActions(possibleActions)
    { /* empty */ }

    virtual bool actionCallback(
            const ActionPtr& action, 
            const ObjectVec& arguments,
            const SIZE_T combinedIndx
            ) noexcept {
        _possibleActions.push_back({action, arguments, combinedIndx});
        return true;
    }
};

} // namespace


QuestApplicableActionsIterator::~QuestApplicableActionsIterator() noexcept 
    = default;
    
bool QuestApplicableActionsIterator::actionCallback(
        const ActionPtr& /*action*/, 
        const ObjectVec& /*arguments*/,
        const SIZE_T /*combinedIndx*/
    ) noexcept
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
    _relevantRelations(buildRelevantRelations(actions)),
    _possibleActions(buildPossibleActions())
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

Quest::PossibleActionVec Quest::buildPossibleActions() const noexcept {
    PossibleActionVec possibleActions;
    PossibleActionsBuilder it(possibleActions);
    Vector<StatementVec> emptyPre;
    iterateOverApplicableActions_Slow(nullState, it, emptyPre);
    return possibleActions;
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

const Quest::PossibleActionVec& Quest::getPossibleActions() const noexcept {
    return _possibleActions;
}

void Quest::iterateOverApplicableActions_Slow(
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
        if(findNextObj(state, it, actionPreBuffers, 
                objects, actionIndx, 0, SIZE_T(0), SIZE_T(1)) == false)
            break; // Stop the search.
    }
}

void Quest::iterateOverApplicableActions(
            const StatePtr& state,
            QuestApplicableActionsIterator& it,
            Vector<StatementVec>& actionPreBuffers
            ) const noexcept {
    for(const ActionWithArgs& aa : _possibleActions) {
        // Objects were selected in such a way that they are suitable by types,
        // but we need to verify if they also satisfy the action preconditions.
        if(state != nullState) {
            if(aa.action->checkActionPreconditions(
                    aa.arguments, state, 
                    actionPreBuffers[aa.combinedIndx % _actions.size()]
                    ) == false)
                continue;
        }
        if(it.actionCallback(
                aa.action, aa.arguments, 
                aa.combinedIndx) == false)
            break;
    }
}

bool Quest::findNextObj(
        const StatePtr& state,
        QuestApplicableActionsIterator& it,
        Vector<StatementVec>& actionPreBuffers,
        ObjectVec &objects,
        ObjectVec::size_type actionIndx,
        ObjectVec::size_type argIndx,
        SIZE_T combinedIndx,
        SIZE_T combinedSize
        ) const noexcept {

    if(argIndx >= _actionArgObjects[actionIndx].size()) {
        // A new possible substitution has been found.
        const ActionPtr& action = _actions[actionIndx];
        // Objects were selected in such a way that they are suitable by types,
        // but we need to verify if they also satisfy the action preconditions.
        if(state != nullState)
            if(action->checkActionPreconditions(
                    objects, state, actionPreBuffers[actionIndx]) == false)
                return true;
        // Call the callback function and return the call's result.
        return it.actionCallback(
                action, objects, 
                actionIndx + combinedIndx * _actions.size());
    }
    const SIZE_T multiplier = _actionArgObjects[actionIndx][argIndx].size();
    for(SIZE_T i = 0; i < multiplier; ++i) {
        const ObjectPtr& obj = _actionArgObjects[actionIndx][argIndx][i];
        bool alreadyInUse = false;
        for(SIZE_T j=0; j<argIndx; ++j)
            if(objects[j]->getId() == obj->getId()) {
                alreadyInUse = true;
                break;
            }
        if(alreadyInUse)
            continue;
        objects[argIndx] = obj;
        if(findNextObj(
                state, it, actionPreBuffers, objects, actionIndx, 
                argIndx + 1, combinedIndx + i * combinedSize,
                combinedSize * multiplier) == false)
            return false;
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
