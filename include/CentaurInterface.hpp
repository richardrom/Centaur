/////////////////////////////////////////////////////////////////////////////////////
//
// Created by Ricardo Romero on 06/12/21.
// Copyright (c) 2021 Ricardo Romero.  All rights reserved.
//

#pragma once

#ifndef __cplusplus
#error "C++ compiler needed"
#endif /*__cplusplus*/

#ifndef CENTAUR_CENTAURINTERFACE_HPP
#define CENTAUR_CENTAURINTERFACE_HPP

#include "Centaur.hpp"

#ifndef DONT_INCLUDE_QT
#include <QIcon>
#include <QString>
#if defined(__clang__) || defined(__GNUC__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wweak-vtables"
#endif /*__clang__*/

#ifndef CENTAUR_INTERFACE_NAMESPACE
#define CENTAUR_INTERFACE_NAMESPACE CENTAUR_NAMESPACE::interface
#endif /*CENTAUR_INTERFACE_NAMESPACE*/

namespace CENTAUR_INTERFACE_NAMESPACE
{
    enum class LogLevel
    {
        fatal, /// Beware that posting a Fatal log will terminate the application
        error,
        info,
        warning,
        trace,
        debug
    };

    enum class AssetImageSource
    {
        Stock,
        Forex,
        Crypto
    };

    /// \brief Handles the logging window
    /// All plugin interfaces accept the ILogger interface to access the Logging window of the main UI
    struct ILogger
    {
        virtual ~ILogger() = default;

        /// \brief Send a log message to the application
        ///
        /// \param source The message source
        /// \param level Level of the diagnosis
        /// \param msg Message
        virtual void log(const QString &source, CENTAUR_INTERFACE_NAMESPACE::LogLevel level, const QString &msg) noexcept = 0;

        /// \brief Wrapper around msg with fatal level
        ///
        /// \param source The message source
        /// \param msg Message
        virtual void fatal(const QString &source, const QString &msg) noexcept = 0;

        /// \brief Wrapper around msg with error level
        ///
        /// \param source The message source
        /// \param msg Message
        virtual void error(const QString &source, const QString &msg) noexcept = 0;

        /// \brief Wrapper around msg with warning level
        ///
        /// \param source The message source
        /// \param msg Message
        virtual void warning(const QString &source, const QString &msg) noexcept = 0;

        /// \brief Wrapper around msg with info level
        ///
        /// \param source The message source
        /// \param msg Message
        virtual void info(const QString &source, const QString &msg) noexcept = 0;

#ifndef NDEBUG
        /// \brief Wrapper around msg with trace level
        ///
        /// \param source The message source
        /// \param msg Message
        virtual void trace(const QString &source, const QString &msg) noexcept = 0;

        /// \brief Wrapper around msg with debug level
        ///
        /// \param source The message source
        /// \param msg Message
        virtual void debug(const QString &source, const QString &msg) noexcept = 0;
#else
        /// \brief Wrapper around msg with trace level
        virtual void trace() noexcept = 0;

        /// \brief Wrapper around msg with debug level
        virtual void debug() noexcept = 0;
#endif /*NDEBUG*/
    };

    /// \brief Provide the methods to access the main configuration file in the plugin data
    ///        In case you want to store an encrypted password you can use the libProtocol interfaces.
    ///        The data must be encrypted with the plugin private key (stored on the Resources/Private folder
    ///        And the plugin must use the public key to decrypt the data
    ///        Configuration files have the ".json" extension
    struct IConfiguration
    {

        enum class CredentialsMethod
        {
            encrypt,
            decrypt
        };
        virtual ~IConfiguration() = default;

    public:
        /// \brief return the configuration filename with json extension
        virtual auto getConfigurationFileName() noexcept -> std::string = 0;

    public:
        /// \brief Ask for the main UI for the Credentials Dialog. Use this to encrypt data
        /// \param textData When Method is encrypt: Text to cipher
        ///                 When Method is decrypt: A Base64 string to decipher
        /// \param forceDialog Set to true if the user credentials dialog must be shown, otherwise,
        ///                     if the main UI holds a copy of the user credentials in memory the dialog will not be shown
        ///                     (it is not guaranteed that the UI will always have such data available)
        /// \param method Encrypt/Decrypt
        /// \throw std::runtime_error in case of the encryption failed. if the user enter bad credentials the function will not throw (see return)
        /// \return When Method is encrypt: A Base64 string with the ciphered data
        ///         When Method is decrypt: A string with the data decrypted.
        ///         In both cases, if the user failed to authenticate the string returned will be empty

        virtual auto credentials(const std::string &textData, bool forceDialog, CredentialsMethod method = CredentialsMethod::encrypt) -> std::string = 0;

        /// Access to images of assets
    public:
        /// \brief Finds the image of the specified asset, size and format (when supported).
        /// \param size Size
        /// \param asset Asset name
        /// \param source Database Origin
        /// \return A null pixmap if it fails, or a loaded pixmap, otherwise.
        /// \remarks The application only supports transparent file formats. In the database files,
        /// the application will search for the files in the order: SVG, PNG, TIFF and GIF for the symbol.
        /// In case the format is not SVG, the application will append the size in the file
        /// for example: supposed the asset named GGG, and the image file exists as a PNG with sizes, 16, 32, 64
        /// The files must be named: GGG_16.png, GGG_32.png, GGG_64.png;
        virtual auto getAssetImage(int size, AssetImageSource source, const QString &asset, QWidget *parent) -> QPixmap = 0;
    };

} // namespace CENTAUR_INTERFACE_NAMESPACE
#endif /*DONT_INCLUDE_QT*/

#if defined(__clang__) || defined(__GNUC__)
#pragma clang diagnostic pop
#endif /*__clang__*/

#endif // CENTAUR_CENTAURINTERFACE_HPP
