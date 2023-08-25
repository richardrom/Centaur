/////////////////////////////////////////////////////////////////////////////////////
//
// Created by Ricardo Romero on 12/06/23.
// Copyright (c) 2023 Ricardo Romero.  All rights reserved.
//

#pragma once

#include "QtCore/qparallelanimationgroup.h"
#include "QtWidgets/qprogressbar.h"
#include "QtWidgets/qstyleoption.h"
#ifndef __cplusplus
#error "C++ compiler needed"
#endif /*__cplusplus*/

#ifndef CENTAUR_CENTTHEME_HPP
#define CENTAUR_CENTTHEME_HPP

#include <Centaur.hpp>
#include <QCheckBox>
#include <QComboBox>
#include <QHeaderView>
#include <QProgressBar>
#include <QPropertyAnimation>
#include <QProxyStyle>
#include <QStateMachine>
#include <QStyleOptionButton>
#include <QTableView>
#include <ThemeInterface.hpp>

namespace helper
{
    /// \brief Each animation base is associated to a QWidget
    struct CENT_LIBRARY_HIDDEN AnimationBase
    {
        explicit AnimationBase(QWidget *parent, const CENTAUR_THEME_INTERFACE_NAMESPACE::AnimationInformation &animInfo);
        virtual ~AnimationBase();

    protected:
        /// \brief Add an animation
        /// \param animInfo Animation information
        void setupAnimation(const CENTAUR_THEME_INTERFACE_NAMESPACE::AnimationInformation &animInfo);

    private:
        QWidget *widget { nullptr };
        std::vector<QStateMachine *> stateMachine;
    };
} // namespace helper

class CENT_LIBRARY_HIDDEN CentTheme : public QProxyStyle
{
    Q_OBJECT
public:
    CentTheme(
        CENTAUR_THEME_INTERFACE_NAMESPACE::ThemeConstants *themeConstants,
        CENTAUR_THEME_INTERFACE_NAMESPACE::ColorScheme *colorScheme,
        CENTAUR_THEME_INTERFACE_NAMESPACE::UIElements *uiElements,
        QPainter::RenderHints renderHints);
    ~CentTheme() override;

    void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const override;
    void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const override;
    void drawComplexControl(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter,
        const QWidget *widget) const override;
    void drawItemText(QPainter *painter, const QRect &rect, int flags, const QPalette &pal, bool enabled, const QString &text,
        QPalette::ColorRole textRole) const override;
    void drawItemPixmap(QPainter *painter, const QRect &rect, int alignment, const QPixmap &pixmap) const override;
    QSize sizeFromContents(ContentsType type, const QStyleOption *option, const QSize &size, const QWidget *widget) const override;
    QRect subElementRect(SubElement element, const QStyleOption *option, const QWidget *widget) const override;
    QRect subControlRect(ComplexControl cc, const QStyleOptionComplex *opt, SubControl sc, const QWidget *widget) const override;
    QRect itemTextRect(const QFontMetrics &fm, const QRect &r, int flags, bool enabled, const QString &text) const override;
    QRect itemPixmapRect(const QRect &r, int flags, const QPixmap &pixmap) const override;
    SubControl hitTestComplexControl(ComplexControl control, const QStyleOptionComplex *option, const QPoint &pos,
        const QWidget *widget) const override;
    int styleHint(StyleHint hint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *returnData) const override;
    int pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const override;
    int layoutSpacing(QSizePolicy::ControlType control1, QSizePolicy::ControlType control2, Qt::Orientation orientation,
        const QStyleOption *option, const QWidget *widget) const override;
    QIcon standardIcon(StandardPixmap standardIcon, const QStyleOption *option, const QWidget *widget) const override;
    QPixmap standardPixmap(StandardPixmap standardPixmap, const QStyleOption *opt, const QWidget *widget) const override;
    QPixmap generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap, const QStyleOption *opt) const override;
    QPalette standardPalette() const override;
    void polish(QWidget *widget) override;
    void polish(QPalette &pal) override;
    void polish(QApplication *app) override;
    void unpolish(QWidget *widget) override;
    void unpolish(QApplication *app) override;

private:
    void drawPushButton(const QStyleOptionButton *option, QPainter *painter, const QWidget *widget) const;
    void drawPushButtonText(const QStyleOptionButton *option, QPainter *painter, const QWidget *widget) const;
    void drawEditLinePanel(const QStyleOption *option, QPainter *painter, const QLineEdit *widget) const;
    void drawComboBox(const QStyleOptionComboBox *option, QPainter *painter, const QComboBox *widget) const;
    void drawPanelMenu(const QStyleOption *option, QPainter *painter) const;
    void drawMenuEmptyArea(const QStyleOption *option, QPainter *painter) const;
    void drawMenuItem(const QStyleOptionMenuItem *option, QPainter *painter, const QWidget *widget) const;
    void drawProgressBarContents(const QStyleOptionProgressBar *option, QPainter *painter, const QProgressBar *widget) const;
    void drawHeaderView(const QStyleOptionHeader *option, QPainter *painter, const QHeaderView *widget) const;
    void drawHeaderSection(const QStyleOptionHeader *option, QPainter *painter, const QHeaderView *widget) const;
    void drawHeaderLabel(const QStyleOptionHeader *option, QPainter *painter, const QHeaderView *widget) const;
    void drawHeaderEmptyArea(const QStyleOption *option, QPainter *painter, const QHeaderView *widget) const;
    void drawTableFrameBackground(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
    void drawTableView(const QStyleOptionViewItem *option, QPainter *painter, const QWidget *widget) const;
    void drawTableViewItemRow(const QStyleOptionViewItem *option, QPainter *painter, const QWidget *widget) const;
    void drawTableViewItem(const QStyleOptionViewItem *option, QPainter *painter, const QWidget *widget) const;
    void drawCheckBox(const QStyleOptionButton *option, QPainter *painter, const QCheckBox *widget) const;
    void drawCheckBoxLabel(const QStyleOptionButton *option, QPainter *painter, const QCheckBox *widget) const;
    void drawCheckBoxIndicator(const QStyleOption *option, QPainter *painter, const QWidget *widget, bool withAnimations = true) const;
    void drawToolButton(const QStyleOptionToolButton *option, QPainter *painter, const QWidget *widget) const;
    void drawToolButtonLabel(const QStyleOptionToolButton *option, QPainter *painter, const QWidget *widget) const;
    void drawGroupBox(const QStyleOptionGroupBox *option, QPainter *painter, const QWidget *widget) const;

    static void drawFrameAnimation(QPainter *painter, const QWidget *widget, const cen::theme::ElementState &state, QRect &widgetRect, bool isSunken = false);

    static void drawFrame(QPainter *painter, const cen::theme::FrameInformation &frameInformation, const QRect &widgetRect);

    auto getPushButtonInformation(const QWidget *widget) const -> CENTAUR_THEME_INTERFACE_NAMESPACE::PushButtonInformation &;
    auto getToolButtonInformation(const QWidget *widget) const -> CENTAUR_THEME_INTERFACE_NAMESPACE::ToolButtonInformation &;
    auto getLineEditInformation(const QWidget *widget) const -> CENTAUR_THEME_INTERFACE_NAMESPACE::LineEditInformation &;
    auto getComboBoxInformation(const QWidget *widget) const -> CENTAUR_THEME_INTERFACE_NAMESPACE::ComboBoxInformation &;

    auto getProgressBarInformation(const QWidget *widget) const -> CENTAUR_THEME_INTERFACE_NAMESPACE::ProgressBarInformation &;
    auto getHeaderInformation(const QWidget *widget, Qt::Orientation orientation) const -> CENTAUR_THEME_INTERFACE_NAMESPACE::HeaderInformation &;
    auto getTableViewInformation(const QWidget *widget) const -> CENTAUR_THEME_INTERFACE_NAMESPACE::TableViewInformation &;
    auto getCheckBoxInformation(const QWidget *widget) const -> CENTAUR_THEME_INTERFACE_NAMESPACE::CheckBoxInformation &;
    auto getGroupBoxInformation(const QWidget *widget) const -> CENTAUR_THEME_INTERFACE_NAMESPACE::GroupBoxInformation &;

    static auto getPushButtonDefaultState(const QStyleOptionButton *option, CENTAUR_THEME_INTERFACE_NAMESPACE::PushButtonInformation *element) -> CENTAUR_THEME_INTERFACE_NAMESPACE::ElementState &;
    static auto getElementState(const QStyleOption *option, CENTAUR_THEME_INTERFACE_NAMESPACE::UIElementBasis *element, bool *validSunken = nullptr) -> CENTAUR_THEME_INTERFACE_NAMESPACE::ElementState &;
    static auto
    getCheckBoxInformationState(const QStyleOption *option, CENTAUR_THEME_INTERFACE_NAMESPACE::CheckBoxInformation *state)
        -> CENTAUR_THEME_INTERFACE_NAMESPACE::CheckBoxInformation::CheckElementState &;

    static auto getComboBoxState(const QStyleOptionComboBox *option, CENTAUR_THEME_INTERFACE_NAMESPACE::ComboBoxInformation *element)
        -> std::tuple<CENTAUR_THEME_INTERFACE_NAMESPACE::ElementState &, QPen, int>;

    static auto initFontFromInfo(const QFont &font_p, const cen::theme::FontTextLayout &fifo) noexcept -> QFont;

    std::unordered_map<QWidget *, std::unique_ptr<helper::AnimationBase>> m_widgetAnimations;

    QPainter::RenderHints m_renderHints;
    CENTAUR_THEME_INTERFACE_NAMESPACE::ThemeConstants *m_themeConstants;
    CENTAUR_THEME_INTERFACE_NAMESPACE::ColorScheme *m_colorScheme;
    CENTAUR_THEME_INTERFACE_NAMESPACE::UIElements *m_uiElements;

    QPixmap m_comboBoxDownArrow { ":/svg/combo-down" };
    QPixmap m_comboBoxDownArrowGray { ":/svg/combo-down-gray" };
    QPixmap m_upArrowWhite { ":/svg/up-arrow-white" };
    QPixmap m_downArrowWhite { ":/svg/down-arrow-white" };
    QIcon m_toolButtonDownArrow { ":/svg/combo-down" };
    QIcon m_menuCheck { ":/svg/check-arrow" };
    QIcon m_menuCheckGray { ":/svg/check-arrow-gray" };
};

#endif // CENTAUR_CENTTHEME_HPP
