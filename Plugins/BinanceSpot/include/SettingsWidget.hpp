/////////////////////////////////////////////////////////////////////////////////////
//
// Created by Ricardo Romero on 20/05/23.
// Copyright (c) 2023 Ricardo Romero.  All rights reserved.
//

#pragma once

#include "CentaurInterface.hpp"
#ifndef __cplusplus
#error "C++ compiler needed"
#endif /*__cplusplus*/

#ifndef CENTAUR_SETTINGSWIDGET_HPP
#define CENTAUR_SETTINGSWIDGET_HPP

#include "Centaur.hpp"
#include <CentaurPlugin.hpp>
#include <QDialog>

BEGIN_CENTAUR_NAMESPACE

namespace Ui
{
    class SettingsWidget;
}

class SettingsWidget : public QWidget
{
public:
    explicit SettingsWidget(CENTAUR_PLUGIN_NAMESPACE::IBase *plugin, CENTAUR_INTERFACE_NAMESPACE::IConfiguration *config, QWidget *parent = nullptr);
    ~SettingsWidget() override;

private:
    struct Impl;
    std::unique_ptr<Impl> _impl;

protected:
    Ui::SettingsWidget *ui();
};

END_CENTAUR_NAMESPACE

#endif // CENTAUR_SETTINGSWIDGET_HPP
