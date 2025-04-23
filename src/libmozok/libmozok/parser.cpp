// Copyright 2025 Pavlo Savchuk. Subject to the MIT license.

#include <libmozok/result.hpp>
#include <libmozok/parser.hpp>
#include <libmozok/error_utils.hpp>

#include <set>

namespace mozok {

RecursiveDescentParser::RecursiveDescentParser(
        const Str& fileName,
        const Str& file,
        const bool prepare) noexcept :
    _text(prepare ? prepareFunc(file + "\n") : file),
    _file(fileName),
    _pos(0),
    _line(0),
    _col(0),
    _src(_text.c_str())
{ /* empty */ }

RecursiveDescentParser::~RecursiveDescentParser()
{ /* empty */ }

Str RecursiveDescentParser::prepareFunc(const Str& src) noexcept {
    Str res;
    bool comment = false;
    for(Str::size_type i=0; i<src.length(); ++i) {
        auto ch = src.at(i);
        if(ch == '#')
            comment = true;
        if(ch == '\r' || ch == '\n')
            comment = false;
        if(comment == false)
            res += ch;
    }
    return res + "\n";
}

Result RecursiveDescentParser::space_symbol() noexcept {
    if(_src[_pos] == '#') {
        while (true) {
            ++_pos;
            ++_col;
            if(_src[_pos] == '\n') return Result::OK();
            if(_src[_pos] == '\0') return Result::OK();
        }
    }
    bool isSpace = false;
    isSpace = isSpace || (_src[_pos] == ' ');
    isSpace = isSpace || (_src[_pos] == '\t');
    if(isSpace == false)
        return errorExpectingSpace(_file, _line, _col);
    _pos++;
    _col++;
    return Result::OK();
}

Result RecursiveDescentParser::space(const int minCount/* = 1*/) noexcept {
    int count = 0;
    for(; count < minCount; ++count) {
        Result res = space_symbol();
        if(res.isError())
            return res;
    }
    while(space_symbol().isOk());
    return Result::OK();
}

Result RecursiveDescentParser::next_line() noexcept {
    if(_src[_pos] == '\n') {
        // LF (Unix)
        ++_pos;
        _col = 0;
        ++_line;
        return Result::OK();
    } else if(_src[_pos] == '\r' && _src[_pos + 1] == '\n') {
        // CRLF (Windows)
        _pos += 2;
        _col = 0;
        ++_line;
        return Result::OK();
    } else if(_src[_pos] == '\r') {
        // CR (old Mac)
        ++_pos;
        _col = 0;
        ++_line;
        return Result::OK();
    } else
        return errorExpectingNewLine(_file, _line, _col);
}

Result RecursiveDescentParser::rest(Str& out) noexcept {
    out = "";
    std::set<char> terminators = {'\0','\n','\r'};
    while(terminators.find(_src[_pos]) == terminators.end()) {
        out += _src[_pos];
        ++_pos;
        ++_col;
    }
    return Result::OK();
}

Result RecursiveDescentParser::empty_lines() noexcept {
    Result res;
    do {space(0);} while (next_line().isOk());
    _pos -= _col;
    _col = 0;
    return Result::OK();
}

Result RecursiveDescentParser::keyword(const char* str) noexcept {
    int len = 0;
    for(; str[len]; ++len)
        if(_src[_pos + len] != str[len])
            return errorExpectingKeyword(_file, _line, _col, str);
    if(_src[_pos + len] == '_' 
            || (_src[_pos + len] >= 'a' && _src[_pos + len] <= 'z')
            || (_src[_pos + len] >= 'A' && _src[_pos + len] <= 'Z'))
        return errorExpectingKeyword(_file, _line, _col, str);
    _pos += len;
    _col += len;
    return Result::OK();
}

Result RecursiveDescentParser::digit() noexcept {
    if(_src[_pos] < '0' || _src[_pos] > '9')
        return errorExpectingDigit(_file, _line, _col);
    ++_pos;
    ++_col;
    return Result::OK();
}

Result RecursiveDescentParser::pos_int(int& out) noexcept {
    const int start = _pos;
    if(digit().isError())
        return errorExpectingDigit(_file, _line, _col);
    while (digit().isOk());
    const int end = _pos;
    out = stoi(_text.substr(start, end - start));
    return Result::OK();
}

Result RecursiveDescentParser::upper_case() noexcept {
    if(_src[_pos] < 'A' || _src[_pos] > 'Z')
        return errorExpectingUppercase(_file, _line, _col);
    ++_pos;
    ++_col;
    return Result::OK();
}

Result RecursiveDescentParser::lower_case() noexcept {
    if(_src[_pos] < 'a' || _src[_pos] > 'z')
        return errorExpectingLowercase(_file, _line, _col);
    ++_pos;
    ++_col;
    return Result::OK();
}

Result RecursiveDescentParser::letter() noexcept {
    if((_src[_pos]>='a' && _src[_pos]<='z') 
            || (_src[_pos]>='A' && _src[_pos]<='Z')) {
        ++_pos;
        ++_col;
        return Result::OK();
    }
    return errorExpectingLetter(_file, _line, _col);
}

Result RecursiveDescentParser::underscore() noexcept {
    if(_src[_pos] != '_')
        return errorExpectingUnderscore(_file, _line, _col);
    ++_pos;
    ++_col;
    return Result::OK();
}

Result RecursiveDescentParser::name(Str& out, const Case first) noexcept {
    Result res;
    const int start = _pos;
    if(first == BOTH) res = letter();
    if(first == UPPER) res = upper_case();
    if(first == LOWER) res = lower_case();
    if(res.isError())
        return res;
    while (true) {
        bool next = false;
        next = next || (letter().isOk());
        next = next || (digit().isOk());
        next = next || (underscore().isOk());
        if(!next) break;
    }
    out = _text.substr(start, _pos - start);
    return Result::OK();
}

Result RecursiveDescentParser::colon() noexcept {
    if(_src[_pos] != ':')
        return errorExpectingColon(_file, _line, _col);
    ++_pos;
    ++_col;
    return Result::OK();
}

Result RecursiveDescentParser::colon_with_spaces() noexcept {
    Result res;
    res <<= space(0);
    res <<= colon();
    res <<= space(0);
    return res;
}

Result RecursiveDescentParser::comma() noexcept {
    if(_src[_pos] != ',')
        return errorExpectingComma(_file, _line, _col);
    ++_pos;
    ++_col;
    return Result::OK();
}

Result RecursiveDescentParser::par_open() noexcept {
    if(_src[_pos] != '(')
        return errorExpectingOpenPar(_file, _line, _col);
    ++_pos;
    ++_col;
    return Result::OK();
}

Result RecursiveDescentParser::par_close() noexcept {
    if(_src[_pos] != ')')
        return errorExpectingClosePar(_file, _line, _col);
    ++_pos;
    ++_col;
    return Result::OK();
}

Result RecursiveDescentParser::bracket_open() noexcept {
    if(_src[_pos] != '[')
        return errorExpectingOpenBracket(_file, _line, _col);
    ++_pos;
    ++_col;
    return Result::OK();
}

Result RecursiveDescentParser::bracket_close() noexcept {
    if(_src[_pos] != ']')
        return errorExpectingOpenBracket(_file, _line, _col);
    ++_pos;
    ++_col;
    return Result::OK();
}

Result RecursiveDescentParser::name_list(StrVec &out, Case firstLetterCase) noexcept {
    Result res;
    while(true) {
        Str newName;
        res <<= empty_lines();
        res <<= space(1);
        res <<= name(newName, firstLetterCase);
        res <<= space(0);
        res <<= next_line();

        if(res.isError()) {
            _pos -= _col;
            _col = 0;
            return Result::OK();
        }

        out.push_back(newName);
    }
    return Result::OK();
}

}
