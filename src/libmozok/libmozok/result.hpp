// Copyright 2024 Pavlo Savchuk. Subject to the MIT license.

#pragma once

#include <libmozok/public_types.hpp>

namespace mozok {

/// @brief Represents the outcome or a status of an operation.
/// Instead of employing the exceptions, libmozok utilizes this structure to 
/// communicate both successful outcomes and encountered errors.
/// You can use the `<<=` operator to merge two results together.
class Result {

    /// @brief Classification of a Result.
    enum ResultType {
        MOZOK_OK, // Indicates that no errors have occurred.
        MOZOK_ERROR, // Indicates that error/errors has/have occurred.
    };

    /// @brief Specific type of the Result.
    /// @see ResultType
    Result::ResultType _type;

    /// @brief This string contains a detailed description of the result.
    /// In a case of an error it provides supplementary error information.
    mozok::Str _description;

public:

    /// @brief The default constructor initializes a result of type `MOZOK_OK` 
    ///     with an empty description.
    Result() noexcept;

    /// @return Returns `true` if no errors have occurred.
    bool isOk() const noexcept;

    /// @return Returns `true` if error/errors has/have occurred.
    bool isError() const noexcept;

    /// @brief This string contains a detailed description of the result.
    ///        In a case of an error it provides supplementary error information.
    /// @return Returns a string that contains a detailed description of the result.
    const Str& getDescription() const noexcept;

    /// @brief `a <<= b` combines two results together into the `a` variable.
    /// When combining two results of type `MOZOK_OK`, the resulting type will 
    /// remain MOZOK_OK. Any other combinations will result in a `MOZOK_ERROR`, 
    /// with a concatenated description. Usage:
    /// @code {.cpp}
    /// Result res = Result::OK();
    /// res <<= Result::OK(); // `res` is still `MOZOK_OK`.
    /// res <<= Result::Error("error_1"); 
    /// // Now `res` is `MOZOK_ERROR` with "error_1" description.
    /// res <<= Result::Error("error_2"); 
    /// // Now `res` is `MOZOK_ERROR` with a concatenated description.
    /// @endcode
    /// @param err 
    /// @return 
    mozok::Result& operator<<=(const mozok::Result& err) noexcept;


    /// @brief Creates a new `MOZOK_OK` result with empty description.
    /// @return Returns a new `MOZOK_OK` result with empty description.
    static mozok::Result OK() noexcept;

    /// @brief Creates a new `MOZOK_ERROR` result with a given description.
    /// @param errorDescription Error description.
    /// @return Creates a new `MOZOK_ERROR` result with a given description.
    static mozok::Result Error(const mozok::Str& errorDescription) noexcept;
};

}