/*
 * Hetero Streams Library - A streaming library for heterogeneous platforms
 * Copyright (c) 2014 - 2016, Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU Lesser General Public License,
 * version 2.1, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 */

#ifndef HSTREAMS_EXCEPTION_H
#define HSTREAMS_EXCEPTION_H

#include <exception>
#include <string>
#include "hStreams_types.h"

/// @brief A customized exception class
///
/// The aim of this class is to carry more information than a simple
/// error message string. E.g. it can carry the error code the thrower
/// of the exception thinks the API should return to the user.
///
/// \sa hStreams_handle_exception()
class hStreams_exception : public std::exception
{
public:
    /// @brief Basic constructor
    /// @param error_code The error code associated with this exception
    /// @note The error message will be a translation of the error code,
    ///     obtained through \c hStreams_ResultGetName()
    hStreams_exception(HSTR_RESULT error_code);
    /// @brief Advanced constructor
    /// @param error_code The error code associated with this exception
    /// @param msg The custom message to be associated with this exception
    hStreams_exception(HSTR_RESULT error_code, std::string const &msg);
    ~hStreams_exception() throw() {};
    const char *what() const throw();
    /// @brief Get the error code associated with the exception
    HSTR_RESULT error_code() const;
private:
    /// @brief A private copy of the error code associated with this exception
    HSTR_RESULT error_code_;
    /// @brief The error message API can present to the user
    std::string msg_;
};

/// @brief A general-purpose exception handler for externally exposed APIs
/// @return Error code to be returned by the API to the user.
///
/// No reason why this cannot be used internally, at least just to emit a message
///
/// This is to be called in the catch-all block of a try-catch, e.g.
/// \code
/// try {
///     do_stuff();
/// } catch (...) {
///     HSTR_RETURN(hStreams_handle_exception());
/// }
/// \endcode
///
/// The internals of this function use rethrow semantics of C++ to analyze
/// the active exception.
HSTR_RESULT hStreams_handle_exception();


#endif /* HSTREAMS_EXCEPTION_H */
