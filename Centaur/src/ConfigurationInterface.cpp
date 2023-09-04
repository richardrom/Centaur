/////////////////////////////////////////////////////////////////////////////////////
//
// Created by Ricardo Romero on 30/05/22.
// Copyright (c) 2022 Ricardo Romero.  All rights reserved.
//

#include "ConfigurationInterface.hpp"
#include "Globals.hpp"
#include "LoginDialog.hpp"
#include <Protocol.hpp>
#include <QApplication>
#include <QSettings>
#include <QStandardPaths>

cen::PluginConfiguration::PluginConfiguration(const QString &uuidString) :
    m_uuidString { uuidString }
{
    const QString localPluginPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/Plugins";

    m_settingsFile = QString("%1/Settings/%2.json").arg(localPluginPath, uuidString);
}

cen::PluginConfiguration::~PluginConfiguration() = default;

auto cen::PluginConfiguration::getConfigurationFileName() noexcept -> std::string
{
    return m_settingsFile.toStdString();
}

auto cen::PluginConfiguration::getAssetImage(int size, CENTAUR_INTERFACE_NAMESPACE::AssetImageSource source, const QString &asset, QWidget *parent) -> QPixmap
{
    return findAssetImage(size, asset, source, parent);
}

auto cen::PluginConfiguration::credentials(const std::string &textData, bool forceDialog, CredentialsMethod method) -> std::string
{
    QSettings settings;
    settings.beginGroup("user.password");
    auto pad = settings.value("pad").toString();
    settings.endGroup();

    settings.beginGroup("local.plugins.iv");
    const auto iv = settings.value(m_uuidString, "").toString();
    settings.endGroup();

    if (iv.isEmpty()) {
        throw std::runtime_error("the plugin was installed with protection disabled");
    }

    const QString cred = [&forceDialog, &pad]() -> QString {
        if (!forceDialog && g_credentials != nullptr && !g_credentials->isEmpty())
            return pad.replace(0, (*g_credentials).size(), *g_credentials);

        LoginDialog dlg(QApplication::activeWindow());
        dlg.setPasswordMode();
        if (dlg.exec() == QDialog::Accepted)
            return pad.replace(0, dlg.userPassword.size(), dlg.userPassword);
        return {};
    }();

    if (cred.isEmpty())
        return {};

    try {
        switch (method) {
            case CredentialsMethod::encrypt:
                return CENTAUR_PROTOCOL_NAMESPACE::Encryption::EncryptAES(textData, cred.toStdString(), iv.toStdString());

            case CredentialsMethod::decrypt:
                return CENTAUR_PROTOCOL_NAMESPACE::Encryption::DecryptAES(textData, cred.toStdString(), iv.toStdString());
        }

    } catch (...) {
        throw;
    }
}
