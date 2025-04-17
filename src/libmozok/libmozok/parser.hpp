#include <libmozok/public_types.hpp>
#include <libmozok/result.hpp>

namespace mozok {

/// @brief Base class for a recursive descent parser.
class RecursiveDescentParser {
protected:
    /// @brief Input file content (with or without the comments).
    const Str _text;

    /// @brief Removes comments.
    static Str prepareFunc(const Str& src) noexcept;

    /// @brief The name of the file.
    const Str _file;

    /// @brief The current position of the parser's cursor within `_src` array.
    int _pos;

    /// @brief The current line of the parser's cursor (starting from 0).
    int _line;

    /// @brief The current column of the parser's cursor (starting from 0).
    int _col;

    /// @brief Source code characters array.
    const char* _src;

public:
    RecursiveDescentParser(
        const Str& fileName,
        const Str& file,
        const bool prepare) noexcept;

    virtual ~RecursiveDescentParser();

    /// @brief Reads a space 'symbol'. Considers commentaries as one space symbol.
    /// @return Returns 'Result::OK()' if the reading operation was successful.
    ///         Otherwise, return a detailed error message.
    Result space_symbol() noexcept;

    /// @brief Parses a span of whitespace.
    /// @param minCount Minimum required amount of 'space' symbols.
    /// @return Returns 'Result::OK()' if the reading operation was successful.
    ///         Otherwise, return a detailed error message.
    Result space(const int minCount/* = 1*/) noexcept;

    /// @brief Parses a next line symbol.
    /// @return Returns 'Result::OK()' if the reading operation was successful.
    ///         Otherwise, return a detailed error message.
    Result next_line() noexcept;

    /// @brief Copies the rest of the line into a string.
    /// @param out Copies the rest of the line into this string.
    /// @return Returns 'Result::OK()' if the reading operation was successful.
    ///         Otherwise, return a detailed error message.
    Result rest(Str& out) noexcept;

    /// @brief Moves the cursor to the next non-empty line.
    /// @return Always returns 'Result::OK()'.
    Result empty_lines() noexcept;

    /// @brief Parses a keyword.
    /// @param str A string containing the keyword.
    /// @return Returns 'Result::OK()' if the reading operation was successful.
    ///         Otherwise, return a detailed error message.
    Result keyword(const char* str) noexcept;

    /// @brief Parses a digit.
    /// @return Returns 'Result::OK()' if the reading operation was successful.
    ///         Otherwise, return a detailed error message.
    Result digit() noexcept;

    /// @brief Parses a non-negative integer number.
    /// @param out The parsed number will be written into this variable.
    /// @return Returns 'Result::OK()' if the reading operation was successful.
    ///         Otherwise, return a detailed error message.
    Result pos_int(int& out) noexcept;

    /// @brief Parses an uppercase letter.
    /// @return Returns 'Result::OK()' if the reading operation was successful.
    ///         Otherwise, return a detailed error message.
    Result upper_case() noexcept;

    /// @brief Parses a lowercase letter.
    /// @return Returns 'Result::OK()' if the reading operation was successful.
    ///         Otherwise, return a detailed error message.
    Result lower_case() noexcept;

    /// @brief Parses a letter.
    /// @return Returns 'Result::OK()' if the reading operation was successful.
    ///         Otherwise, return a detailed error message.
    Result letter() noexcept;

    /// @brief Parses an underscore '_' character.
    /// @return Returns 'Result::OK()' if the reading operation was successful.
    ///         Otherwise, return a detailed error message.
    Result underscore() noexcept;

    /// @brief Letter case - uppercase, lowercase or both.
    enum Case {
        BOTH, // Upper and lower case.
        UPPER, // Uppercase.
        LOWER, // Lowercase.
    };

    /// @brief Parse a name. Name must start from a letter.
    /// @param out The parsed name will be written into this variable.
    /// @param first The case of the first letter (`UPPER`, `LOWER`, `BOTH`).
    /// @return Returns 'Result::OK()' if the reading operation was successful.
    ///         Otherwise, return a detailed error message.
    Result name(Str& out, const Case first = BOTH) noexcept;

    /// @brief Parses a colon ':' symbol.
    /// @return Returns 'Result::OK()' if the reading operation was successful.
    ///         Otherwise, return a detailed error message.
    Result colon() noexcept;

    /// @brief Parses a colon that may be surrounded by spaces.
    /// @return Returns 'Result::OK()' if the reading operation was successful.
    ///         Otherwise, return a detailed error message.
    Result colon_with_spaces() noexcept;

    /// @brief Parses a comma ',' symbol.
    /// @return Returns 'Result::OK()' if the reading operation was successful.
    ///         Otherwise, return a detailed error message.
    Result comma() noexcept;
    
    /// @brief Parses a open parenthesis '(' symbol.
    /// @return Returns 'Result::OK()' if the reading operation was successful.
    ///         Otherwise, return a detailed error message.
    Result par_open() noexcept;

    /// @brief Parses a closed parenthesis ')' symbol.
    /// @return Returns 'Result::OK()' if the reading operation was successful.
    ///         Otherwise, return a detailed error message.
    Result par_close() noexcept;

    /// @brief Parses a open bracket '[' symbol.
    /// @return Returns 'Result::OK()' if the reading operation was successful.
    ///         Otherwise, return a detailed error message.
    Result bracket_open() noexcept;

    /// @brief Parses a closed bracket ']' symbol.
    /// @return Returns 'Result::OK()' if the reading operation was successful.
    ///         Otherwise, return a detailed error message.
    Result bracket_close() noexcept;

    /// @brief Parses a vertical list of names.
    /// @param out The parsed list will be added into this variable.
    /// @param firstLetterCase First letter case (`UPPER`, `LOWER`, `BOTH`).
    /// @return Returns 'Result::OK()' if the reading operation was successful.
    ///         Otherwise, return a detailed error message.
    Result name_list(StrVec &out, Case firstLetterCase) noexcept;

};

}

