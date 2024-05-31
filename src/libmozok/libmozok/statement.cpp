// Copyright 2024 Pavlo Savchuk. Subject to the MIT license.

#include <libmozok/statement.hpp>

namespace mozok {

std::size_t StatementHash::operator()(
        const StatementPtr& statement) const noexcept {
    return statement->getHash();
}

bool StatementEqual::operator()(
        const StatementPtr& a, 
        const StatementPtr& b
        ) const noexcept {
    #ifdef MOZOK_OUTPUT_HASH_COLLISIONS_INFO
    static int ccounter = 0;
    #endif
    if(a->getHash() != b->getHash())
        return false;
    if(a->getRelation() != b->getRelation()) {
        #ifdef MOZOK_OUTPUT_HASH_COLLISIONS_INFO
        ccounter++;
        printf("StatePtr: collision Type1 %d!\n", ccounter);
        #endif
        return false;
    }
    const ObjectVec argA = a->getArguments();
    const ObjectVec argB = b->getArguments();
    if(argA.size() != argB.size()) {
        #ifdef MOZOK_OUTPUT_HASH_COLLISIONS_INFO
        ccounter++;
        printf("StatePtr: collision Type2 %d!\n", ccounter);
        #endif
        return false;
    }
    for(ObjectVec::size_type i = 0; i < argA.size(); ++i)
        if(argA[i] != argB[i]) {
            #ifdef MOZOK_OUTPUT_HASH_COLLISIONS_INFO
            ccounter++;
            printf("StatePtr: collision Type3 %d!\n", ccounter);
            #endif
            return false;
        }
    return true;
}


Statement::Statement(
        const RelationPtr& relation, 
        const ObjectVec& arguments
        ) noexcept :
    _relation(relation),
    _arguments(arguments),
    _isConstant(isConstant(arguments)),
    _isGlobal(isGlobal(arguments)),
    _hash(computeHash())
{ /* empty */ }
    
bool Statement::isConstant() const noexcept {
    return _isConstant;
}

bool Statement::isGlobal() const noexcept {
    return _isGlobal;
}

StatementPtr Statement::substitute(const ObjectVec& arguments) const noexcept {
    ObjectVec resArguments;
    for(ObjectVec::size_type indx = 0; indx < _arguments.size(); ++indx) {
        const ObjectPtr& arg = _arguments[indx];
        if(arg->getId() >= ID(0))
            resArguments.push_back(arg);
        else
            resArguments.push_back(arguments[ID(-1) - arg->getId()]);
    }
    return makeShared<Statement>(_relation, resArguments);
}

bool Statement::isConstant(const ObjectVec& arguments) const noexcept {
    for(const ObjectPtr& arg : arguments)
        if(arg->getId() < ID(0))
            return false;
    return true;
}

bool Statement::isGlobal(const ObjectVec& arguments) const noexcept {
    // Zero-arity relation is considered global.
    if(arguments.size() == 0)
        return true;
    for(const ObjectPtr& arg : arguments)
        if(arg->getId() >= ID(0))
            return true;
    return false;
}

Result Statement::checkArgumentsCompatibility(
        const ObjectVec& arguments) const noexcept {
    return _relation->checkArgumentsCompatibility(arguments);
}

const RelationPtr& Statement::getRelation() const noexcept {
    return _relation;
}

ObjectVec& Statement::getArguments() noexcept {
    return _arguments;
}

const ObjectVec& Statement::getArguments() const noexcept {
    return _arguments;
}

std::size_t Statement::computeHash() const noexcept {
    std::size_t result = std::hash<ID>{}(_relation->getId());
    const ObjectVec& args = getArguments();
    for(ObjectVec::size_type i = 0; i < args.size(); ++i) {
        result += std::hash<ID>{}(
                // 10007 and 100003 are both prime numbers.
                _relation->getId() + i * 10007 + args[i]->getId() * 100003);
    }
    return result;
}

std::size_t Statement::getHash() const noexcept {
    return _hash;
}

void Statement::recalculateHash() noexcept {
    _hash = computeHash();
}

}