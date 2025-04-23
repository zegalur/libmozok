// Copyright 2024 Pavlo Savchuk. Subject to the MIT license.

#include <libmozok/relation_list.hpp>
#include <libmozok/error_utils.hpp>

namespace mozok {

RelationList::RelationList(
        const Str& name,
        const ID id,
        const ObjectVec& arguments, 
        const StatementVec& statements
        ) noexcept :
    _name(name),
    _id(id),
    _arguments(arguments),
    _statements(statements)
{ /* empty */ }

const StatementVec& RelationList::getStatements() const noexcept {
    return _statements;
}

const ObjectVec& RelationList::getArguments() const noexcept {
    return _arguments;
}

const Str& RelationList::getName() const noexcept {
    return _name;
}

ID RelationList::getId() const noexcept {
    return _id;
}

Result RelationList::checkArgumentsCompatibility(
        const ObjectVec& arguments) const noexcept {
    if(arguments.size() != _arguments.size())
        return errorRListArgError_InvalidArity(
                _name, (int)_arguments.size(), (int)arguments.size());

    for(ObjectVec::size_type i = 0; i < arguments.size(); ++i) {
        const ObjectPtr arg = arguments[i];
        const TypeSet argTypeSet = arg->getTypeSet();
        const TypeSet rlistArgTypeSet = _arguments[i]->getTypeSet();

        bool include = true;
        for(const TypePtr& type : rlistArgTypeSet)
            if(argTypeSet.find(type) == argTypeSet.end()) {
                include = false;
                break;
            }

        if(include == false)
            return errorRListArgError_InvalidType(
                    _name, int(i), arg->getName(), 
                    typesetToStrVec(argTypeSet),
                    typesetToStrVec(rlistArgTypeSet));
    }
    return Result::OK();
}

StatementVec RelationList::substitute(const ObjectVec& arguments) const noexcept {
    StatementVec res;
    for(const StatementPtr& statement : _statements)
        if(statement->isConstant())
            res.push_back(statement);
        else
            res.push_back(statement->substitute(arguments));
    return res;
}

void RelationList::substituteFast(
        StatementVec& out, const ObjectVec& arguments) const noexcept {
    for(StatementVec::size_type i=0; i<_statements.size(); ++i) {
        if(_statements[i]->isConstant() == false) {
            // Instead of calling the `Statement::substitute(...)` method:
            //  out[i] = _statements[i]->substitute(arguments);
            // we perform in-place substitution for optimization purposes.
            const ObjectVec& stArgs = _statements[i]->getArguments();
            ObjectVec& outArgs = out[i]->getArguments();
            for(ObjectVec::size_type indx = 0; indx < stArgs.size(); ++indx) {
                const ObjectPtr& arg = stArgs[indx];
                if(arg->getId() < ID(0))
                    outArgs[indx] = arguments[ID(-1) - arg->getId()];
            }
            out[i]->recalculateHash();
        }
    }
}

}
