/////////////////////////////////////////////////////////////////////////////////////
//
// Created by Ricardo Romero on 10/02/23.
// Copyright (c) 2023 Ricardo Romero.  All rights reserved.
//

#pragma once

#ifndef __cplusplus
#error "C++ compiler needed"
#endif /*__cplusplus*/

#ifndef CENTAUR_DAL_HPP
#define CENTAUR_DAL_HPP

#include <Centaur.hpp>
#include <QException>
#include <QWidget>

namespace CENTAUR_NAMESPACE::dal
{
    enum class OpenDatabaseCode
    {
        Ok,      //! Database open
        Fatal,   //! Failed to create the database
        Recreate //! Recreate the database
    };

    struct PluginData
    {
        inline PluginData(QString nm, QString vr, QString mn, QString id, QString uiv, QString chk, QString dyn, bool en, bool pr) :
            name { std::move(nm) },
            version { std::move(vr) },
            man { std::move(mn) },
            uuid { std::move(id) },
            centaur_uuid { std::move(uiv) },
            checksum { std::move(chk) },
            dynamic { std::move(dyn) },
            enabled { en },
            protect { pr } { }

        QString name;
        QString version;
        QString man;
        QString uuid;
        QString centaur_uuid;
        QString checksum;
        QString dynamic;
        bool enabled;
        bool protect;
    };

    struct DataAccess final
    {
        /// \brief Open the database
        /// \param parent Parent Widget for QMessageBox internal calls
        /// \return Actions to take after the call
        static OpenDatabaseCode openDatabase(QWidget *parent = nullptr) noexcept;

        /// \brief Creates the database an all its queries
        static OpenDatabaseCode createDatabase(QWidget *parent = nullptr) noexcept;

        /// \brief Determinate if a plugin exists
        /// \param uuid Plugin Universal Unique Identifier
        /// \return The optional value holds True if exists or False if it doesn't. A std::nullopt will be returned if the internal query fails
        static std::optional<bool> pluginExists(const QString &uuid) noexcept;

        /// \brief Get a plugin data
        /// \param uuid Plugin Universal Unique Identifier
        /// \return The optional value holds the plugin data. A std::nullopt will be returned if the internal query fails
        static std::optional<PluginData> pluginInformation(const QString &uuid) noexcept;

        /// \brief Retrieve all plugin information
        static std::optional<QList<PluginData>> pluginInformation() noexcept;

        /// \brief Update enabling state
        /// \param uuid Plugin Universal Unique Identifier
        /// \return The optional value holds True if the field was actually updated.  A std::nullopt will be returned if the internal query fails
        static std::optional<bool> updateEnabledState(const QString &uuid, bool newState) noexcept;

        /// \brief Inserts a new plugin into the database. The function does not do any previous checks
        /// \param name Plugin Name
        /// \param version Plugin version string
        /// \param man Plugin manufacturer String
        /// \param uuid Universal Unique Identifier
        /// \param centaur_uuid UUID of the most recent version of the UI that is supported by the plugin
        /// \param checksum Plugin Sha224 checksum
        /// \param dynamic Name of the dylib/so/dll file that contains the plugin executable
        /// \param enabled True if the plugin is enabled at startup, false otherwise
        /// \param protect True if the plugin needs
        /// \return The optional value holds True if exists or False if it doesn't. A std::nullopt will be returned if the internal query fails
        static std::optional<bool> insertPlugin(const QString &name,
            const QString &version,
            const QString &man,
            const QString &uuid,
            const QString &centaur_uuid,
            const QString &checksum,
            const QString &dynamic,
            bool enabled,
            bool protect) noexcept;

        /// \brief Get the name of the dylib/so/dll file that contains the plugin executable
        /// \param uuid Plugin Universal Unique Identifier
        /// \return The optional value holds True if the field was actually updated. A std::nullopt will be returned if the internal query fails
        static std::optional<QString> getDynamicFieldPlugin(const QString &uuid) noexcept;

        /// \brief Removes a plugin only from the database
        /// \param uuid Plugin Universal Unique Identifier
        /// \return The optional value holds True if the plugin was removed. A std::nullopt will be returned if the internal query fails
        static std::optional<bool> removePlugin(const QString &uuid) noexcept;

        /// \brief Gets the version string of a plugin
        /// \param uuid Plugin Universal Unique Identifier
        /// \return The optional value holds the version string. A std::nullopt will be returned if the internal query fails
        static std::optional<QString> fromUUIDtoVersion(const QString &uuid) noexcept;

        /// \brief Add a symbol to the favorites watchlist
        /// \param symbol Symbol name
        /// \param pluginUUID Symbol Exchange plugin source
        /// \return The optional value holds True if the plugin was added. A std::nullopt will be returned if the internal query fails
        static std::optional<bool> addSymbolToFavorites(const QString &symbol, const QString &pluginUUID);

        /// \brief Retrive all symbols in the favorites watchlist
        /// \return The optional value holds a vector of all symbols paired by Symbol (first) and Source (second). A std::nullopt will be returned if the internal query fails
        static std::optional<std::vector<std::pair<QString, QString>>> selectFavoriteSymbols();

        /// \brief Delete a symbol from the favorites list
        /// \param symbol Symbol name
        /// \param pluginUUID Symbol exchange plugin source
        /// \return The optional value holds True if the row was removed. A std::nullopt will be returned if the internal query fails
        static std::optional<bool> deleteFavoritesSymbol(const QString &symbol, const QString &pluginUUID);
    };

} // namespace CENTAUR_NAMESPACE::dal

#endif // CENTAUR_DAL_HPP
