// Copyright 2024 Pavlo Savchuk. Subject to the MIT license.

#include <libmozok/state.hpp>
#include <libmozok/quest.hpp>

namespace mozok {

std::size_t StateHash::operator()(const StatePtr& state) const noexcept {
    return state->getHash();
}

bool StateEqual::operator()(const StatePtr& a, const StatePtr& b) const noexcept {
    #ifdef MOZOK_OUTPUT_HASH_COLLISIONS_INFO
    static int ccounter = 0;
    #endif

    if(a->getHash() != b->getHash())
        return false;
    const StatementSet& aSet = a->getStatementSet();
    const StatementSet& bSet = b->getStatementSet();
    if(aSet.size() != bSet.size()) {
        #ifdef MOZOK_OUTPUT_HASH_COLLISIONS_INFO
        ccounter++;
        printf("StatePtr: collision Type1 %d!\n", ccounter);
        #endif
        return false;
    }
    for(const StatementPtr& statement : aSet)
        if(bSet.find(statement) == bSet.end()) {
            #ifdef MOZOK_OUTPUT_HASH_COLLISIONS_INFO
            ccounter++;
            printf("StatePtr: collision Ttype2 %d!\n\n", ccounter);

            printf("%lld == %lld\n\n", a->getHash(), b->getHash());
            printf("A set:\n");
            for(const StatementPtr& statement : aSet)
                printf("%s(%s, %s) : %lld\n", 
                    statement->getRelation()->getName().c_str(),
                    statement->getArguments()[0]->getName().c_str(),
                    statement->getArguments()[1]->getName().c_str(),
                    statement->getHash());
            printf("\n\n");
            printf("B set:\n");
            for(const StatementPtr& statement : bSet)
                printf("%s(%s, %s) : %lld\n", 
                    statement->getRelation()->getName().c_str(),
                    statement->getArguments()[0]->getName().c_str(),
                    statement->getArguments()[1]->getName().c_str(),
                    statement->getHash());
            printf("\n\n");
            #endif

            return false;
        }
    return true;
}


State::State(const StatementVec& statements) noexcept :
    _hash(computeHash()) {
    addStatements(statements);
}

const StatementSet& State::getStatementSet() const noexcept {
    return _state;
}

bool State::hasSubstate(const StatementVec& substate) const noexcept {
    for(const StatementPtr& statement : substate)
        if(_state.find(statement) == _state.end())
            return false;
    return true;
}

void State::addStatements(const StatementVec& statements) noexcept {
    for(const StatementPtr& statement : statements)
        if(_state.find(statement) == _state.end()) {
            _state.insert(statement);
            _hash ^= statement->getHash();
        }
}

void State::removeStatements(const StatementVec& statements) noexcept {
    for(const StatementPtr& statement : statements)
        if(_state.find(statement) != _state.end()) {
            _state.erase(statement);
            _hash ^= statement->getHash();
        }
}

std::size_t State::computeHash() const noexcept {
    std::size_t result = 0;
    for(const StatementPtr& statement : _state)
        result ^= statement->getHash();
    return result;
}

std::size_t State::getHash() const noexcept {
    return _hash;
}

StatePtr State::duplicate() const noexcept {
    StatePtr res = makeShared<State>(StatementVec());
    res->_hash = _hash;
    res->_state = _state;
    return res;
}

StatePtr State::duplicate(const Quest& quest) const noexcept {
    StatePtr res = makeShared<State>(StatementVec());
    for(const StatementPtr& statement : _state) {
        bool relevant = quest.isRelationRelevant(
                statement->getRelation()->getId());
        if(relevant)
            for(const ObjectPtr& obj : statement->getArguments())
                if(quest.isObjectRelevant(obj->getId()) == false) {
                    relevant = false;
                    break;
                }
        if(relevant)
            res->_state.insert(statement);
    }
    res->_hash = res->computeHash();
    return res;
}

}