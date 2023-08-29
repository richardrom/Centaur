/////////////////////////////////////////////////////////////////////////////////////
//
// Created by Ricardo Romero on 12/06/23.
// Copyright (c) 2023 Ricardo Romero.  All rights reserved.
//

#include <QSettings>
#include <memory>
#include <stdexcept>
#include <utility>

#include "CentTheme.hpp"
#include "ThemeParser.hpp"
#include "ThemePlugin.hpp"

CentThemePlugin::CentThemePlugin() :
    m_availableSchemes { "Dark", "Light" } { }

bool CentThemePlugin::loadTheme(const QString &scheme) noexcept
{
    try {
        if (scheme == "dark") {
            m_parser.loadTheme(QString(m_extraPath + "/dark.theme.xml").toStdString());
        }
        else if (scheme == "light") {
            m_parser.loadTheme(QString(m_extraPath + "/light.theme.xml").toStdString());
        }

        if (!m_parser.getErrors().empty()) {
            for (const auto &error : m_parser.getErrors()) { qDebug() << error; }
        }
    } catch (
#ifndef DEBUG
        C_UNUSED
#endif /*DEBUG*/
        const std::runtime_error &ex) {
#ifdef DEBUG
        qDebug() << ex.what();
#endif /*DEBUG*/

        // On non-debug compilations it will silently return
        return false;
    }
    return true;
}

void CentThemePlugin::accessExtra(const QString &pluginExtraPath) noexcept
{
    m_extraPath = pluginExtraPath;

#ifdef Q_OS_WIN
    QChar pathSeparator = '\\';
#else
    QChar pathSeparator = '/';
#endif

    // Append the UUID to the directory
    if (!m_extraPath.isEmpty() && m_extraPath.back() != pathSeparator)
        m_extraPath.append(pathSeparator);
    m_extraPath.append(QString("%2").arg(uuid()));

    QSettings settings("CentaurProject", "CentTheme");
    settings.beginGroup("color.scheme");

    m_currentColorScheme = settings.value("name", "dark").toString();

    loadTheme(m_currentColorScheme);

    settings.endGroup();
}

QString CentThemePlugin::themeName() const noexcept { return { "Centaur Main Theme" }; }

QString CentThemePlugin::uuid() const noexcept { return "2df95e88-7eae-5940-ab0d-b53f2df855e2"; }

QWidget *CentThemePlugin::settingsWidget() const noexcept { return nullptr; }

QStringList CentThemePlugin::colorSchemes() const noexcept { return m_availableSchemes; }

void CentThemePlugin::setColorScheme(const QString &newColorScheme) noexcept
{
    const QString newColorWithoutCase = newColorScheme.toLower();

    if (m_currentColorScheme == newColorWithoutCase)
        return;

    QSettings settings("CentaurProject", "CentTheme");

    const QString tempThemeName = [&]() -> QString {
        for (const auto &scheme : std::as_const(m_availableSchemes)) {
            if (scheme.toLower() == newColorWithoutCase) {
                settings.setValue("name", scheme);
                return scheme;
            }
        }
        return "";
    }();

    if (tempThemeName.isEmpty())
        return;

    settings.beginGroup("color.scheme");
    if (loadTheme(tempThemeName))
        m_currentColorScheme = tempThemeName;
    settings.setValue("name", m_currentColorScheme);
    settings.endGroup();
}

QString CentThemePlugin::colorSchemeName() const noexcept { return m_currentColorScheme; }

const CENTAUR_THEME_INTERFACE_NAMESPACE::ColorScheme &CentThemePlugin::colorScheme() const noexcept { return m_parser.scheme; }

const CENTAUR_THEME_INTERFACE_NAMESPACE::UIElements &CentThemePlugin::uiElements() const noexcept { return m_parser.uiElements; }

QPainter::RenderHints CentThemePlugin::renderHint() const noexcept { return m_parser.renderHints; }

QStyle *CentThemePlugin::create(const QString &key)
{
    if (key.toLower() == "centheme") {
        return new CentTheme(
            std::addressof(m_parser.constants),
            std::addressof(m_parser.scheme),
            std::addressof(m_parser.uiElements),
            m_parser.renderHints);
    }
    return nullptr;
}
