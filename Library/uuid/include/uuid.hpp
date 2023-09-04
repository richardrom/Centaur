/////////////////////////////////////////////////////////////////////////////////////
//
// Created by Ricardo Romero on 13/01/22.
// Copyright (c) 2022 Ricardo Romero.  All rights reserved.
//

#pragma once

#ifndef __cplusplus
#error "C++ compiler needed"
#endif /*__cplusplus*/

#ifndef CENTAUR_UUID_HPP
#define CENTAUR_UUID_HPP

#include "Centaur.hpp"
#include <array>
#include <concepts>
#include <random>
#include <span>
#include <string>

#ifdef QT_VERSION
#include <QString>
#include <QUuid>
#define QT_INCLUDED
#endif /*QT_VERSION*/

#define CENTAUR_VALID_UUID_GENERATORS std::same_as<std::mt19937, generator> || std::same_as<std::mt19937_64, generator> || std::same_as<std::minstd_rand, generator> || std::same_as<std::minstd_rand0, generator> || std::same_as<std::ranlux24, generator> || std::same_as<std::ranlux48, generator>

#ifndef CENTAUR_UUID_ALIGNMENT
#if defined(__clang__) || defined(__GNUC__)
#define CENTAUR_UUID_ALIGNMENT __attribute__((aligned(sizeof(uint64_t))))
#elif defined MSVC
#define CENTAUR_UUID_ALIGNMENT __declspec(align(8))
#else
#define CENTAUR_UUID_ALIGNMENT
#endif /*defined(__clang__) || defined(__GNUC__)*/
#endif /*CENTAUR_UUID_ALIGNMENT*/

namespace CENTAUR_NAMESPACE
{
    /// \brief A general UUID class to describe a 128-bit Universal Unique Identifiers
    struct uuid
    {
        /// \brief Construct an uuid from a string
        /// \param str The string must have the dashes
        /// \param checkForBrackets Check for the { } at the beginning and at the end of the string
        explicit uuid(std::string str, bool checkForBrackets = true);

        /// \brief Generates a uuid with {u1-w1-w2-b1b2b3b4b5b6}
        uuid(uint32_t u1, uint16_t w1, uint16_t w2, uint16_t w3, uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5, uint8_t b6, bool endiannessCheck = true) noexcept;

        // Copy constructor
        uuid(const uuid &id);

        // Move constructor
        uuid(uuid &&id) noexcept;

        uuid &operator=(const uuid &id);

        uuid();

    protected:
        uuid(uint8_t *raw, std::size_t size);

    public:
        /// \brief Returns the string bytes
        /// \return A lower case of the uuid string
        C_NODISCARD auto to_string(bool brackets = true, bool upper = false) const -> std::string;

#ifdef QT_INCLUDED
        C_NODISCARD auto to_qstring(bool brackets = true, bool upper = false) const -> QString
        {
            return QString::fromStdString(to_string(brackets, upper));
        }

        C_NODISCARD auto to_quuid(bool upper = false) const -> QUuid
        {
            // Brackets are mandatory according to QUuid documentation
            return QUuid::fromString(to_qstring(true, upper));
        }
#endif

        /// \brief Generates a random UUID
        /// \return A random UUID

        C_NODISCARD static auto generate() -> cen::uuid;

        C_NODISCARD inline const uint8_t *bytes() const
        {
            return data.data();
        }

        C_NODISCARD inline auto view() const -> auto
        {
            return data;
        }

    private:
        std::array<uint8_t, 16> data;

        friend bool operator==(const uuid &id1, const uuid &id2) noexcept;

    public:
        inline bool operator<(const uuid &id) const
        {
            return data < id.data;
        }
    };

    bool operator==(const uuid &id1, const uuid &id2) noexcept;

    bool operator==(const uuid &id1, const std::string &str) noexcept;

#ifdef QT_INCLUDED
    inline bool operator==(const uuid &id1, const QString &str) noexcept
    {
        try {
            return id1 == str.toStdString();
        } catch (C_UNUSED const std::exception &ex) {
            return false;
        }
    }
#endif

} // namespace CENTAUR_NAMESPACE

#ifndef C_NO_UUID_HASH
namespace std
{
    template <>
    struct hash<CENTAUR_NAMESPACE::uuid>
    {
        inline std::size_t operator()(const CENTAUR_NAMESPACE::uuid &id) const
        {
            const auto view = id.view();

            auto _u64_1 = (static_cast<uint64_t>(view.at(0)) << 56) + (static_cast<uint64_t>(view.at(1)) << 48) + (static_cast<uint64_t>(view.at(2)) << 40) + (static_cast<uint64_t>(view.at(3)) << 32) + (static_cast<uint64_t>(view.at(4)) << 24) + (static_cast<uint64_t>(view.at(5)) << 16) + (static_cast<uint64_t>(view.at(6)) << 8) + static_cast<uint64_t>(view.at(7));
            auto _u64_2 = (static_cast<uint64_t>(view.at(8)) << 56) + (static_cast<uint64_t>(view.at(9)) << 48) + (static_cast<uint64_t>(view.at(10)) << 40) + (static_cast<uint64_t>(view.at(11)) << 32) + (static_cast<uint64_t>(view.at(12)) << 24) + (static_cast<uint64_t>(view.at(13)) << 16) + (static_cast<uint64_t>(view.at(14)) << 8) + static_cast<uint64_t>(view.at(15));

            std::hash<uint64_t> hs;
            return hs(_u64_1) ^ hs(_u64_2);
        }
    };
} // namespace std
#endif /*C_NO_UUID_HASH*/

#endif // CENTAUR_UUID_HPP
