/////////////////////////////////////////////////////////////////////////////////////
//
// Created by Ricardo Romero on 22/11/22.
// Copyright (c) 2022 Ricardo Romero.  All rights reserved.
//

// IBase Implementation

#include "BinanceSPOT.hpp"
#include "SettingsWidget.hpp"
#include <QFile>

namespace
{
    /// UUID V5 string "CentaurProject-BinanceExchangeSPOT-0.1.0"
    constexpr char g_BinanceSpotName[]          = "BinanceSPOT";
    constexpr char g_BinanceSpotVersionString[] = "0.1.0";
    constexpr char g_uuidString[]               = "85261bc6-8f92-57ca-802b-f08b819031db";
} // namespace

QObject *CENTAUR_NAMESPACE::BinanceSpotPlugin::getPluginObject() noexcept
{
    return qobject_cast<QObject *>(this);
}

QString CENTAUR_NAMESPACE::BinanceSpotPlugin::getPluginVersionString() const noexcept
{
    return g_BinanceSpotVersionString;
}

QString CENTAUR_NAMESPACE::BinanceSpotPlugin::getPluginName() const noexcept
{
    return g_BinanceSpotName;
}

void CENTAUR_NAMESPACE::BinanceSpotPlugin::setPluginInterfaces(CENTAUR_INTERFACE_NAMESPACE::ILogger *logger, CENTAUR_INTERFACE_NAMESPACE::IConfiguration *config) noexcept
{
    m_logger = logger;
    m_config = config;
}

CENTAUR_NAMESPACE::uuid CENTAUR_NAMESPACE::BinanceSpotPlugin::getPluginUUID() const noexcept
{
    static uuid globalPluginUuid { g_uuidString, false };
    return globalPluginUuid;
}

QString CENTAUR_NAMESPACE::BinanceSpotPlugin::getUUIDString() const noexcept
{
    return g_uuidString;
}

QWidget *CENTAUR_NAMESPACE::BinanceSpotPlugin::settingsWidget(IBase *thisObject, CENTAUR_INTERFACE_NAMESPACE::IConfiguration *config) const noexcept
{
    return new SettingsWidget(thisObject, config);
}

void CENTAUR_NAMESPACE::BinanceSpotPlugin::storeData() noexcept
{
    rapidjson::StringBuffer jsonBuffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(jsonBuffer);
    pluginSettings.Accept(writer);

    QFile file(QString::fromStdString(m_config->getConfigurationFileName()));
    if (file.open(QFile::WriteOnly | QFile::Truncate))
    {
        QTextStream stream(&file);
        stream << jsonBuffer.GetString();
        file.close();
    }
}

void CENTAUR_NAMESPACE::BinanceSpotPlugin::updateKeys(const QString &api, const QString &secret) noexcept
{
    m_keys.apiKey    = api.toStdString();
    m_keys.secretKey = secret.toStdString();

    try
    {
        auto permissions = m_bAPI->getAPIKeyPermission();
        if (permissions.createTime > 0)
            m_validKeys = true;
    } catch (C_UNUSED const BINAPI_NAMESPACE::APIException &ex)
    {
        m_validKeys = false;
    }

    emit displayChange(DisplayRole::Foreground);
}
