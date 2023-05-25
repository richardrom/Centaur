/////////////////////////////////////////////////////////////////////////////////////
//
// Created by Ricardo Romero on 30/05/22.
// Copyright (c) 2022 Ricardo Romero.  All rights reserved.
//

#pragma once

#ifndef __cplusplus
#error "C++ compiler needed"
#endif /*__cplusplus*/

#ifndef CENTAUR_CONFIGURATIONINTERFACE_HPP
#define CENTAUR_CONFIGURATIONINTERFACE_HPP

#include "Centaur.hpp"
#include "CentaurPlugin.hpp"

#include <unordered_map>

BEGIN_CENTAUR_NAMESPACE

class PluginConfiguration : public CENTAUR_NAMESPACE::interface::IConfiguration
{
public:
    explicit PluginConfiguration(const QString &uuidString);
    ~PluginConfiguration() override;

public:
    auto getConfigurationFileName() noexcept -> std::string override;
    auto credentials(const std::string &textData, bool forceDialog, CredentialsMethod method = CredentialsMethod::encrypt) -> std::string override;

public:
    auto getAssetImage(int size, CENTAUR_INTERFACE_NAMESPACE::AssetImageSource source, const QString &asset, QWidget *parent) -> QPixmap override;

private:
    QString m_path;
    QString m_settingsFile;
    QString m_uuidString;
};

END_CENTAUR_NAMESPACE

#endif // CENTAUR_CONFIGURATIONINTERFACE_HPP
