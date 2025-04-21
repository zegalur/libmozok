// Copyright 2024 Pavlo Savchuk. Subject to the MIT license.

#include <libmozok/message_processor.hpp>
#include <libmozok/action.hpp>
#include <libmozok/error_utils.hpp>

namespace mozok {

Action::Action(
        const Str& name, 
        const ID id, 
        const bool isNotApplicable,
        const ObjectVec arguments,
        const StatementVec pre,
        const StatementVec rem,
        const StatementVec add
        ) noexcept :
    _name(name),
    _id(id),
    _isNotApplicable(isNotApplicable),
    _arguments(arguments),
    _pre("_pre", ID(-1), arguments, pre),
    _rem("_rem", ID(-1), arguments, rem),
    _add("_add", ID(-1), arguments, add),
    _isGlobal(calculateLocality())
{ /* empty */ }

bool Action::calculateLocality() const noexcept {
    for(const RelationList* rlist : {&_pre, &_rem, &_add})
        for(const StatementPtr& st : rlist->getStatements())
            if(st->isGlobal())
                return true;
    return false;
}

const Str& Action::getName() const noexcept {
    return _name;
}

ID Action::getId() const noexcept {
    return _id;
}

bool Action::isNotApplicable() const noexcept {
    return _isNotApplicable;
}

const ObjectVec& Action::getArguments() const noexcept {
    return _arguments;
}

const RelationList& Action::getPreconditions() const noexcept {
    return _pre;
}

const RelationList& Action::getRemList() const noexcept {
    return _rem;
}

const RelationList& Action::getAddList() const noexcept {
    return _add;
}

bool Action::isGlobal() const noexcept {
    return _isGlobal;
}

Result Action::evaluateActionApplicability(
        const bool doNotCheckPreconditions,
        const ObjectVec& arguments,
        const StatePtr& state,
        ActionError& actionError
        ) const noexcept {
    if(arguments.size() != _arguments.size()) {
        actionError = MOZOK_AE_ARITY_ERROR;
        return errorActionArgError_InvalidArity(
                _name, (int)_arguments.size(), (int)arguments.size());
    }
    
    for(ObjectVec::size_type i = 0; i < arguments.size(); ++i) {
        const ObjectPtr& object = arguments[i];
        const ObjectPtr& argument = _arguments[i];
        if(!areTypesetsCompatible(object->getTypeSet(), argument->getTypeSet())) {
            actionError = MOZOK_AE_TYPE_ERROR;
            return errorActionArgError_InvalidType(
                    _name, i, object->getName(),
                    typesetToStrVec(object->getTypeSet()),
                    typesetToStrVec(argument->getTypeSet()));
        }
    }

    if(doNotCheckPreconditions == false) {
        const StatementVec preconditions = _pre.substitute(arguments);
        if(state->hasSubstate(preconditions) == false) {
            actionError = MOZOK_AE_PRECONDITIONS_ERROR;
            return errorActionPreconditionsFailed(Str("???"), _name);
        }
    }
    
    actionError = MOZOK_AE_NO_ERROR;
    return Result::OK();
}

Result Action::applyAction(
        const ObjectVec& arguments, 
        StatePtr& state,
        ActionError &actionError
        ) const noexcept {
    Result res = evaluateActionApplicability(
            false, arguments, state, actionError);
    if(res.isError())
        return res;

    state->removeStatements(_rem.substitute(arguments));
    state->addStatements(_add.substitute(arguments));
    return res;
}

void Action::applyActionUnsafe(
            const ObjectVec& arguments, 
            StatePtr& state
            ) const noexcept {
    state->removeStatements(_rem.substitute(arguments));
    state->addStatements(_add.substitute(arguments));
}

bool Action::checkActionPreconditions(
        const ObjectVec& arguments, 
        const StatePtr& state,
        StatementVec& preBuffer
        ) const noexcept {
    _pre.substituteFast(preBuffer, arguments);
    if(state->hasSubstate(preBuffer) == false)
        return false;
    return true;
}

}
