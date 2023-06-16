/////////////////////////////////////////////////////////////////////////////////////
//
// Created by Ricardo Romero on 04/06/23.
// Copyright (c) 2023 Ricardo Romero.  All rights reserved.
//

#pragma once

#include "QtCore/qeasingcurve.h"
#ifndef __cplusplus
#error "C++ compiler needed"
#endif /*__cplusplus*/

#ifndef CENTAUR_THEMEPLUGIN_HPP
#define CENTAUR_THEMEPLUGIN_HPP

#include "Centaur.hpp"
#include <QBrush>
#include <QColor>
#include <QEasingCurve>
#include <QPainter>
#include <QStylePlugin>

#ifndef CENTAUR_THEME_INTERFACE_NAMESPACE
#define CENTAUR_THEME_INTERFACE_NAMESPACE CENTAUR_NAMESPACE::theme
#endif /*CENTAUR_THEME_INTERFACE_NAMESPACE*/

#ifndef BEGIN_CENTAUR_THEME_NAMESPACE
#define BEGIN_CENTAUR_THEME_NAMESPACE           \
    namespace CENTAUR_THEME_INTERFACE_NAMESPACE \
    {
#endif

#ifndef END_CENTAUR_THEME_NAMESPACE
#define END_CENTAUR_THEME_NAMESPACE }
#endif

BEGIN_CENTAUR_THEME_NAMESPACE

struct ThemeAnimation
{
    QEasingCurve easingCurve;
    int duration;
};

using ColorMap     = std::unordered_map<QString, QColor>;
using BrushMap     = std::unordered_map<QString, QBrush>;
using PenMap       = std::unordered_map<QString, QPen>;
using AnimationMap = std::unordered_map<QString, ThemeAnimation>;

/// \brief The theme color palette
/// This structure holds every color and/or brush used by all Widgets in the Centaur UI interface
/// It also holds the values for the in-house derived Widgets
struct ColorScheme
{
    ColorMap colors;
    BrushMap brushes;
    PenMap pens;
};

struct UIElements
{
    AnimationMap animations;
};

#if defined(C_GNU_CLANG)
CENTAUR_WARN_PUSH()
CENTAUR_WARN_OFF("-Wweak-vtables")
#endif /*__clang__*/

/// \brief A description of a theme. <br>
/// About the icons: A theme will not hold the icons for any tradeable symbol. This functionality is reserved for the main UI implementation itself
/// only UI general icons will be hold and accessed through a theme. See \ref colorSchemeName for more details.<br>
/// Currently all themes can be pack via the plpack tool.
struct ITheme : public QStylePlugin
{
    ~ITheme() override = default;

public:
    /// \brief After loading the theme plugin this function will be called from the UI to inform the plugin where the extra data path is located
    /// \param pluginExtraPath The local path where the data is located
    /// \code
    /// static QString global_data_path;
    /// class UuidThemeImpl : public cen::theme::CtTheme
    /// {
    /// public:
    ///    inline void accessExtra(const QString &pluginExtraPath) const noexcept override
    ///    {
    ///       global_data_path = pluginExtraData;
    ///    }
    /// };
    /// \endcode
    /// \remarks This is the first function that is call from the UI so this can be used as an initialization process
    virtual void accessExtra(const QString &pluginExtraPath) noexcept = 0;

    /// \brief Access the plugin theme name that implements
    /// \return A QString with the theme name
    C_NODISCARD virtual QString themeName() const noexcept = 0;

    /// \brief A Universal Unique Identifier for the plugin
    /// \return A string with a 128-bit UUID.
    /// \warning Beware that the string will be passed to a \ref cen::uuid class so
    /// make sure the string complies with a standard UUID format without the curly braces
    /// \code
    /// class UuidThemeImpl : public cen::theme::CtTheme
    /// {
    /// public:
    ///    inline QString uuid() const noexcept override
    ///    {
    ///        return "00000000-0000-0000-0000-000000000000";
    ///    }
    /// };
    /// \endcode
    C_NODISCARD virtual QString uuid() const noexcept = 0;

    /// \brief This function is intended to return a settings widget for the theme
    /// \return A valid QWidget to be displayed in the settings page or a nullptr if no page is going to be displayed
    C_NODISCARD virtual QWidget *settingsWidget() const noexcept = 0;

    /// \brief A list of all color schemes that plugin implements
    /// \return A QString list with the color scheme implemented by the theme
    C_NODISCARD virtual QStringList colorSchemes() const noexcept = 0;

    /// \brief Set the current color scheme for the UI
    /// \param newColorScheme A color scheme name that is compatible with the ones returned by \ref colorSchemes()
    /// \remarks Name looking should be done ignoring the case
    virtual void setColorScheme(const QString &newColorScheme) noexcept = 0;

    /// \brief Access the current color scheme. Assuming that the theme can implement several color schemes like a dark and, a light color scheme within the theme
    /// \remarks In general, a UI plugin is not aware of the color schemes, but can access the current color scheme of the theme. <br>
    /// So use keywords in the color scheme name to hint the plugins about the general color theme, for example, Dark Theme or Light Theme <br>
    /// In this way a plugin can display icons properly according to the current color scheme from the current theme
    /// \return A QString with color scheme name
    C_NODISCARD virtual QString colorSchemeName() const noexcept = 0;

    /// \brief Access the color scheme object of the theme
    /// \return A ColorScheme const reference
    C_NODISCARD virtual const ColorScheme &colorScheme() const noexcept = 0;

    /// \brief Return the main UI Elements of the theme
    /// \return A cref to UIElements struct
    C_NODISCARD virtual const UIElements &uiElements() const noexcept = 0;

    /// \brief Return the plugin general render hint. It is not mandatory that every QPainter object be under this scheme
    /// \return A set of QPainter::RenderHint OR'ed values
    C_NODISCARD virtual int renderHint() const noexcept = 0;
};

#if defined(C_GNU_CLANG)
CENTAUR_WARN_POP()
#endif

END_CENTAUR_THEME_NAMESPACE

#define ITheme_iid "com.centaur-project.plugin.ITheme_iid/1.0"
Q_DECLARE_INTERFACE(CENTAUR_THEME_INTERFACE_NAMESPACE::ITheme, ITheme_iid)

#endif // CENTAUR_THEMEPLUGIN_HPP
