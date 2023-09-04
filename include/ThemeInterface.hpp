/////////////////////////////////////////////////////////////////////////////////////
//
// Created by Ricardo Romero on 04/06/23.
// Copyright (c) 2023 Ricardo Romero.  All rights reserved.
//

#pragma once

#include "QtCore/qeasingcurve.h"
#include "QtCore/qmargins.h"
#include "QtCore/qnamespace.h"
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

/// \brief At what element to apply the animation
enum class AnimationElement
{
    Border,              /// \brief Apply to the pen color
    Background,          /// \brief Apply to the brush color
    uCheckBoxBackground, /// \brief Apply to the brush color of the checkbox (unchecked)
    uCheckBoxBorder,     /// \brief Apply to the pen color of the checkbox (unchecked)
    cCheckBoxBackground, /// \brief Apply to the brush color of the checkbox (checked)
    cCheckBoxBorder,     /// \brief Apply to the pen color of the checkbox (checked)
};

/// \brief What kind of animation
enum class AnimationApplication
{
    HoverIn,         /// \brief Apply the animation element to the UI hover in
    HoverOut,        /// \brief Apply the animation element to the UI hover out
    FocusIn,         /// \brief Apply the animation element to the UI focus in
    FocusOut,        /// \brief Apply the animation element to the UI focus out
    CheckBoxFocusIn, /// \brief Apply the animation to the hover-in checkbox
    CheckBoxFocusOut /// \brief Apply the animation to the hover-out checkbox
};

struct ThemeAnimation
{
    QEasingCurve easingCurve;
    int duration;
};

struct FrameInformation
{
    QMargins margins { 0, 0, 0, 0 };
    QMargins padding { 0, 0, 0, 0 };
    qreal borderRadiusX { 0 };
    qreal borderRadiusY { 0 };
    qreal borderRadiusTopLeft { 0 };
    qreal borderRadiusTopRight { 0 };
    qreal borderRadiusBottomLeft { 0 };
    qreal borderRadiusBottomRight { 0 };
    QPen leftBorderPen { Qt::NoPen };
    QPen rightBorderPen { Qt::NoPen };
    QPen bottomBorderPen { Qt::NoPen };
    QPen topBorderPen { Qt::NoPen };
};

struct FontStyle
{
    QString fontName;
    qreal size { -1.0 };
    qreal letterSpacing { -1.0 };
    qreal wordSpacing { -1.0 };
    int weight { QFont::Weight::Normal };
    int stretchFactor { QFont::AnyStretch };
    QFont::Capitalization caps { QFont::Capitalization::MixedCase };
    QFont::SpacingType spacingType { QFont::SpacingType::PercentageSpacing };
    bool italic { false };
    bool kerning { false };
    bool underline { false };
};

struct FontTextLayout
{
    QTextOption opts;
    FontStyle style;
};

struct ElementState
{
    QPen pen { Qt::NoPen };
    QPen fontPen { Qt::NoPen };
    QBrush brush { Qt::NoBrush };
    FrameInformation fi;
    FontTextLayout fontInformation;
};

/// \brief Steps information
using AnimationSteps = std::unordered_map<AnimationApplication, ThemeAnimation *>;

/// \brief Information data
struct Animation
{
    AnimationElement element;
    QVariant start;
    QVariant end;
    AnimationSteps animationSteps;
};

/// \brief Animation type
using AnimationInformation = std::vector<Animation>;

struct Elements
{
    ElementState normal;
    ElementState hover;
    ElementState focus;
    ElementState pressed;
};

struct UIElementBasis
{
    AnimationInformation animations;
    Elements enabled;
    Elements disabled;
};

struct PushButtonInformation : public UIElementBasis
{
    Elements defaultButton;
};

struct LineEditInformation : public UIElementBasis
{
    QColor textColor;
    QColor disableTextColor;
    QColor placeHolderColor;
};

struct ComboBoxInformation : public UIElementBasis
{
    QPen dropArrowPen { Qt::NoPen };
    int dropArrowSize { -1 };
};

struct MenuInformation
{
    QBrush panelBrush { Qt::NoBrush };
    QBrush emptyAreaBrush { Qt::NoBrush };
    QBrush separatorBrush { Qt::NoBrush };
    QBrush selectedBrush { Qt::NoBrush };

    QPen separatorPen { Qt::NoPen };
    QPen selectedPen { Qt::NoPen };
    QPen enabledPen { Qt::NoPen };
    QPen disabledPen { Qt::NoPen };

    FontTextLayout selectedFont;
    FontTextLayout enabledFont;
    FontTextLayout disabledFont;

    int itemHeight { -1 };
    int separatorHeight { -1 };
    int leftPadding { 0 };
};

struct ProgressBarInformation
{
    bool applyGlowEffect { false };

    struct EffectInformation
    {
        QColor glowColor;
        qreal xOffset;
        qreal yOffset;
        qreal blurRadius;
    } effectInformation;

    FrameInformation fi;
    QBrush disableGrooveBrush { Qt::NoBrush };
    QBrush enabledGrooveBrush { Qt::NoBrush };
    QBrush disableBarBrush { Qt::NoBrush };
    QBrush enabledBarBrush { Qt::NoBrush };
};

struct HeaderInformation
{
    QBrush emptyAreaBrush { Qt::NoBrush };
    QBrush backgroundBrush { Qt::NoBrush };
    QBrush hoverBrush { Qt::NoBrush };
    QBrush sunkenBrush { Qt::NoBrush };
    QBrush disableBackgroundBrush { Qt::NoBrush };
    QBrush disableEmptyAreaBrush { Qt::NoBrush };

    QPen sectionLinesPen { Qt::NoPen };
    QPen disableSectionLinesPen { Qt::NoPen };

    QPen disableFontPen { Qt::NoPen };
    QPen fontPen { Qt::NoPen };
    QPen hoverPen { Qt::NoPen };
    QPen sunkenPen { Qt::NoPen };

    FontTextLayout disableFont {};
    FontTextLayout font {};
    FontTextLayout hoverFont {};
    FontTextLayout sunkenFont {};

    QMargins sectionLinesMargins { 0, 0, 0, 0 };

    bool showSectionLines { true };
};

struct CheckBoxInformation
{
    AnimationInformation animations;

    struct CheckElementState
    {
        FrameInformation widgetFrame;
        QBrush widgetBrush { Qt::NoBrush };
        QPen widgetPen { Qt::NoPen };

        FrameInformation checkedBoxFrame;
        QBrush checkedBoxBrush { Qt::NoBrush };
        QPen checkedBoxPen { Qt::NoPen };
        QPen checkedBoxIndicatorPen { Qt::NoPen };
        QPen uncheckedFontPen { Qt::NoPen };
        FontTextLayout uncheckedFont {};

        FrameInformation uncheckedBoxFrame;
        QBrush uncheckedBoxBrush { Qt::NoBrush };
        QPen uncheckedBoxPen { Qt::NoPen };
        QPen uncheckedBoxIndicatorPen { Qt::NoPen };
        QPen checkedFontPen { Qt::NoPen };
        FontTextLayout checkedFont {};

        FrameInformation undefinedBoxFrame;
        QBrush undefinedBoxBrush { Qt::NoBrush };
        QPen undefinedBoxPen { Qt::NoPen };
        QPen undefinedBoxIndicatorPen { Qt::NoPen };
        QPen undefinedFontPen { Qt::NoPen };
        FontTextLayout undefinedFont {};
    };

    struct CheckElements
    {
        CheckElementState normal;
        CheckElementState hover;
        CheckElementState focus;
    };

    CheckElements disabled;
    CheckElements enabled;
};

struct TableViewInformation
{
    QBrush paneBackgroundBrush { Qt::NoBrush };
    QBrush backgroundBrush { Qt::NoBrush };
    QBrush itemAltBackgroundBrush { Qt::NoBrush };
    QBrush itemBackgroundBrush { Qt::NoBrush };
    QBrush disableBackgroundBrush { Qt::NoBrush };
    QBrush itemFocusBrush { Qt::NoBrush };
    QBrush itemHoverBrush { Qt::NoBrush };
    QBrush itemSelectedBrush { Qt::NoBrush };

    QPen gridLinesPen { Qt::NoPen };

    QPen disabledPen { Qt::NoPen };
    QPen fontPen { Qt::NoPen };
    QPen focusPen { Qt::NoPen };
    QPen selectedPen { Qt::NoPen };
    QPen hoverPen { Qt::NoPen };

    FontTextLayout disabledFont {};
    FontTextLayout fontFont {};
    FontTextLayout focusFont {};
    FontTextLayout selectedFont {};
    FontTextLayout hoverFont {};

    QMargins itemMargins { 0, 0, 0, 0 };

    bool mouseOverItem { false };
    bool gridLines { false };
};

struct GroupBoxInformation
{
    FrameInformation fi;
    QBrush contentsBrush { Qt::NoBrush };
    QBrush headerBrush { Qt::NoBrush };
    QBrush indicatorBrush { Qt::NoBrush };
    QBrush disabledIndicatorBrush { Qt::NoBrush };
    QPen pen { Qt::NoPen };
    QPen disabledPen { Qt::NoPen };
    FontTextLayout font;
    FontTextLayout disabledFont;
    int indicatorWidth { 0 };
    int headerHeight { -1 };
};

struct DialogInformation
{
    QBrush backgroundBrush { Qt::NoBrush };
    QPen borderPen { Qt::NoPen };
    FrameInformation frameInformation;
};

struct TitleBarInformation
{
    QBrush backgroundBrush { Qt::NoBrush };
    FrameInformation frameInformation;
    FontTextLayout font;
    QPen fontPen;
};

using ToolButtonInformation = PushButtonInformation;

using ColorMap            = std::unordered_map<QString, QColor>;
using BrushMap            = std::unordered_map<QString, QBrush>;
using PenMap              = std::unordered_map<QString, QPen>;
using FontStyleMap        = std::unordered_map<QString, FontTextLayout>;
using AnimationMap        = std::unordered_map<QString, ThemeAnimation>;
using FramesMap           = std::unordered_map<QString, FrameInformation>;
using ButtonMap           = std::unordered_map<QString, PushButtonInformation>;
using ToolButtonMap       = std::unordered_map<QString, ToolButtonInformation>;
using LineEditMap         = std::unordered_map<QString, LineEditInformation>;
using ComboBoxMap         = std::unordered_map<QString, ComboBoxInformation>;
using ProgressBarMap      = std::unordered_map<QString, ProgressBarInformation>;
using VerticalHeaderMap   = std::unordered_map<QString, HeaderInformation>;
using HorizontalHeaderMap = std::unordered_map<QString, HeaderInformation>;
using TableViewMap        = std::unordered_map<QString, TableViewInformation>;
using CheckBoxMap         = std::unordered_map<QString, CheckBoxInformation>;
using GroupBoxMap         = std::unordered_map<QString, GroupBoxInformation>;
using DialogMap           = std::unordered_map<QString, DialogInformation>;
using TitleBarMap         = std::unordered_map<QString, TitleBarInformation>;

struct ThemeConstants
{
    QSize menuItemImage { 0, 0 };
    std::unordered_map<QString, int> treeItemHeight;
};

/// \brief The theme color palette
/// This structure holds every color and/or brush used by all Widgets in the Centaur UI interface
/// It also holds the values for the in-house derived Widgets
struct ColorScheme
{
    ColorMap colors;
    BrushMap brushes;
    PenMap pens;
    FontStyleMap fonts;
};

struct UIElements
{
    AnimationMap animations;
    FramesMap frames;
    ButtonMap pushButtonOverride;
    ToolButtonMap toolButtonOverride;
    LineEditMap lineEditOverride;
    ComboBoxMap comboBoxOverride;
    ProgressBarMap progressBarOverride;
    VerticalHeaderMap verticalHeaderOverride;
    HorizontalHeaderMap horizontalHeaderOverride;
    TableViewMap tableViewOverride;
    CheckBoxMap checkBoxOverride;
    GroupBoxMap groupBoxOverride;
    DialogMap dialogOverride;
    TitleBarMap titleBarOverride;
    PushButtonInformation pushButtonInformation;
    ToolButtonInformation toolButtonInformation;
    LineEditInformation lineEditInformation;
    ComboBoxInformation comboBoxInformation;
    MenuInformation menuInformation;
    ProgressBarInformation progressBarInformation;
    HeaderInformation verticalHeaderInformation;
    HeaderInformation horizontalHeaderInformation;
    TableViewInformation tableViewInformation;
    CheckBoxInformation checkBoxInformation;
    GroupBoxInformation groupBoxInformation;
    DialogInformation dialogInformation;
    TitleBarInformation titleBarInformation;
};

#if defined(C_GNU_CLANG)
CENTAUR_WARN_PUSH()
CENTAUR_WARN_OFF("-Wweak-vtables")
#endif /*__clang__*/

/// \brief A description of a theme. <br>
/// About the icons: A theme will not hold the icons for any tradeable symbol. This functionality is reserved for the main UI
/// implementation itself only UI general icons will be hold and accessed through a theme. See \ref colorSchemeName for more
/// details.<br> Currently all themes can be pack via the plpack tool.
struct ITheme : public QStylePlugin
{
    ~ITheme() override = default;

public:
    /// \brief After loading the theme plugin this function will be called from the UI to inform the plugin where the extra data path
    /// is located \param pluginExtraPath The local path where the data is located \code static QString global_data_path; class
    /// UuidThemeImpl : public cen::theme::CtTheme
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

    /// \brief Access the current color scheme. Assuming that the theme can implement several color schemes like a dark and, a light
    /// color scheme within the theme \remarks In general, a UI plugin is not aware of the color schemes, but can access the current
    /// color scheme of the theme. <br> So use keywords in the color scheme name to hint the plugins about the general color theme, for
    /// example, Dark Theme or Light Theme <br> In this way a plugin can display icons properly according to the current color scheme
    /// from the current theme \return A QString with color scheme name
    C_NODISCARD virtual QString colorSchemeName() const noexcept = 0;

    /// \brief Access the color scheme object of the theme
    /// \return A ColorScheme const reference
    C_NODISCARD virtual const ColorScheme &colorScheme() const noexcept = 0;

    /// \brief Return the main UI Elements of the theme
    /// \return A cref to UIElements struct
    C_NODISCARD virtual const UIElements &uiElements() const noexcept = 0;

    /// \brief Return the plugin general render hint. It is not mandatory that every QPainter object be under this scheme
    /// \return A set of QPainter::RenderHint OR'ed values
    C_NODISCARD virtual QPainter::RenderHints renderHint() const noexcept = 0;
};

#if defined(C_GNU_CLANG)
CENTAUR_WARN_POP()
#endif

END_CENTAUR_THEME_NAMESPACE

#define ITheme_iid "com.centaur-project.plugin.ITheme_iid/1.0"
Q_DECLARE_INTERFACE(CENTAUR_THEME_INTERFACE_NAMESPACE::ITheme, ITheme_iid)

#endif // CENTAUR_THEMEPLUGIN_HPP
