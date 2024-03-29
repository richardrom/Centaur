////////////////////////////////////////////////////////////////////////////////
// Created by Ricardo Romero on 21/2/2022 for BinanceAPI.
// Copyright (c) 2022. Ricardo Romero
// All rights reserved.
////////////////////////////////////////////////////////////////////////////////

#ifndef __cplusplus
#error "C++ compiler needed"
#endif /*__cplusplus*/

#pragma once

#ifndef APIEXCEPTION_HPP
#define APIEXCEPTION_HPP

#include "BinanceAPIGlobal.hpp"
#include <exception>
#include <optional>
#include <string>
#include <tuple>

namespace BINAPI_NAMESPACE
{
    /// Exception type
    using xstr = std::optional<const char *>;
    using xint = std::optional<int>;

    ///
    /// \brief The APIException class
    /// Throw after some error processing an API request
    ///
    class BINAPI_EXPORT APIException final : public std::exception
    {
    public:
        enum class Type
        {
            request,
            limits,
            http1,
            http2,
            json,
            api,
            schema
        };

    public:
        // Use for limits
        APIException(Type type, std::string what) noexcept;

        // Use for Type::request, Type::http1 and Type::api
        APIException(Type type, int code, cpr::Url url, std::string what) noexcept;

        // Use for Type::http2
        APIException(Type type, long status, int code, cpr::Url url, std::string what, std::string msg) noexcept;

        // Use for type::json and type::schema
        APIException(Type type, cpr::Url url, std::string what) noexcept;

        ~APIException() override;

    public:
        /// \brief what All what messages
        /// \return usually the what message
        T_NODISCARD const char *what() const noexcept override;

        /// \brief url Get the url
        /// \return An optional URL value
        T_NODISCARD xstr url() const noexcept;

        /// \brief msg Gets the HTTP2 message from the BinanceAPI
        /// \return An optional message
        T_NODISCARD xstr msg() const noexcept;

        /// \brief status Gets the HTTP2 status code
        /// \return an optional status code
        T_NODISCARD xint status() const noexcept;

        /// \brief code An error code
        /// \return The code of the request or the HTTPx api code
        T_NODISCARD xint code() const noexcept;

        /// \brief type Get type
        /// \return The exception type
        T_NODISCARD Type type() const noexcept;

    public:
        T_NODISCARD std::tuple<std::string> limits() const noexcept;

        /// \brief Return the request exception data
        /// \return {code, URL, message}
        T_NODISCARD std::tuple<int, std::string, std::string> request() const noexcept;

        /// \brief Return a http1 exception data
        /// \return {status, URL, what}
        T_NODISCARD std::tuple<int, std::string, std::string> http1() const noexcept;

        /// \brief Return an http2 exception data
        /// \return {status, code, URL, reason, message}
        T_NODISCARD std::tuple<int, int, std::string, std::string, std::string> http2() const noexcept;

        /// \brief Return a JSON exception data
        /// \return {URL, what}
        T_NODISCARD std::tuple<std::string, std::string> json() const noexcept;

        /// \brief Return an API exception data
        /// \return  {Code, URL, what}
        T_NODISCARD std::tuple<int, std::string, std::string> api() const noexcept;

        /// \brief Return a schema exception data
        /// \return {URL, what}
        T_NODISCARD std::tuple<std::string, std::string> schema() const noexcept;

    private:
        Type m_type;
        cpr::Url m_url;
        std::string m_what;
        std::string m_msg;
        long m_status { 0 };
        int m_code { 0 };
    };
} // namespace trader::api

#endif // APIEXCEPTION_HPP
