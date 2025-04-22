// Copyright 2024 Pavlo Savchuk. Subject to the MIT license.

#include <libmozok/result.hpp>

namespace mozok {

namespace {

/// @brief Maximum length of result (error) description. 
///        Any description exceeding this limit will be automatically truncated.
const int MOZOK_MAX_ERROR_LENGTH = 10000;

}

Result::Result() noexcept : _type(MOZOK_OK)
{ /* empty */ }

Result Result::OK() noexcept {
    Result res;
    res._type = MOZOK_OK;
    res._description = "";
    return res;
}

Result Result::Error(const mozok::Str& errorDescription) noexcept {
    Result res;
    res._type = MOZOK_ERROR;
    // TODO: make the colored 'error' label optional.
    //const Str errorLabel = "\033[1;31merror: \033[0m";
    const Str errorLabel = "error: ";
    res._description = errorLabel + errorDescription + "\n";
    return res;
}

Result& Result::operator<<=(const Result& err) noexcept {
    if(err._type == MOZOK_ERROR) {
        _type = MOZOK_ERROR;
        if(_description.length() > MOZOK_MAX_ERROR_LENGTH) {
            _description = _description.substr(0, MOZOK_MAX_ERROR_LENGTH);
            if(_description.at(_description.length() - 1) != '\n')
                _description += "...\n";
            _description += "...\n... [Too many errors! ";
            _description += "Next error is the most recent error]\n...\n";
        }
        _description += err._description;
    }
    return *this;
}

bool Result::isOk() const noexcept {
    return _type == MOZOK_OK;
}

bool Result::isError() const noexcept {
    return _type == MOZOK_ERROR;
}

const Str& Result::getDescription() const noexcept {
    return _description;
}

}
