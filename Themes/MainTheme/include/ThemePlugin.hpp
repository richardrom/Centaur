/////////////////////////////////////////////////////////////////////////////////////
//
// Created by Ricardo Romero on 12/06/23.
// Copyright (c) 2023 Ricardo Romero.  All rights reserved.
//

#pragma once

#ifndef __cplusplus
#error "C++ compiler needed"
#endif /*__cplusplus*/

#ifndef CENTAUR_THEME_HPP
#define CENTAUR_THEME_HPP

#include "ThemeParser.hpp"
#include <Centaur.hpp>
#include <ThemeInterface.hpp>

class CENT_LIBRARY CentThemePlugin : public CENTAUR_THEME_INTERFACE_NAMESPACE::ITheme
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.centaur-project.plugin.CentThemePlugin/1.0")
    Q_INTERFACES(CENTAUR_THEME_INTERFACE_NAMESPACE::ITheme)
public:
    CentThemePlugin();
    ~CentThemePlugin() override = default;

    void accessExtra(const QString &pluginExtraPath) noexcept override;
    C_NODISCARD QString themeName() const noexcept override;
    C_NODISCARD QString uuid() const noexcept override;
    C_NODISCARD QWidget *settingsWidget() const noexcept override;
    C_NODISCARD QStringList colorSchemes() const noexcept override;
    void setColorScheme(const QString &newColorScheme) noexcept override;
    C_NODISCARD QString colorSchemeName() const noexcept override;
    C_NODISCARD const cen::theme::ColorScheme &colorScheme() const noexcept override;
    C_NODISCARD const cen::theme::UIElements &uiElements() const noexcept override;
    C_NODISCARD QPainter::RenderHints renderHint() const noexcept override;
    QStyle *create(const QString &key) override;

private:
    bool loadTheme(const QString &scheme) noexcept;

private:
    QStringList m_availableSchemes;
    QString m_currentColorScheme;
    QString m_extraPath;
    theme::ThemeParser m_parser;
};

#endif // CENTAUR_THEME_HPP
