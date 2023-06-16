/////////////////////////////////////////////////////////////////////////////////////
//
// Created by Ricardo Romero on 12/06/23.
// Copyright (c) 2023 Ricardo Romero.  All rights reserved.
//

#pragma once

#ifndef __cplusplus
#error "C++ compiler needed"
#endif /*__cplusplus*/

#ifndef CENTAUR_CENTTHEME_HPP
#define CENTAUR_CENTTHEME_HPP

#include <Centaur.hpp>
#include <QPropertyAnimation>
#include <QProxyStyle>
#include <QStateMachine>
#include <ThemeInterface.hpp>

namespace helper
{
    struct HoverAnimation
    {
        HoverAnimation(
            QObject *parent,
            QWidget *widget,
            const QEasingCurve &easingHover,
            const QEasingCurve &easingLeave,
            const QColor &initialStateColor,
            const QColor &endStateColor,
            int hoverEnterDuration,
            int hoverLeaveDuration);
        ~HoverAnimation();

        QStateMachine *machine { nullptr };
        QState *stateEnter { nullptr };
        QState *stateLeave { nullptr };
    };
} // namespace helper

class CENT_LIBRARY_HIDDEN CentTheme : public QProxyStyle
{
    Q_OBJECT
public:
    explicit CentTheme(CENTAUR_THEME_INTERFACE_NAMESPACE::ColorScheme *colorScheme, CENTAUR_THEME_INTERFACE_NAMESPACE::UIElements *uiElements);
    ~CentTheme() override;

    void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const override;
    void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const override;
    void drawComplexControl(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const override;
    void drawItemText(QPainter *painter, const QRect &rect, int flags, const QPalette &pal, bool enabled, const QString &text, QPalette::ColorRole textRole) const override;
    void drawItemPixmap(QPainter *painter, const QRect &rect, int alignment, const QPixmap &pixmap) const override;
    QSize sizeFromContents(ContentsType type, const QStyleOption *option, const QSize &size, const QWidget *widget) const override;
    QRect subElementRect(SubElement element, const QStyleOption *option, const QWidget *widget) const override;
    QRect subControlRect(ComplexControl cc, const QStyleOptionComplex *opt, SubControl sc, const QWidget *widget) const override;
    QRect itemTextRect(const QFontMetrics &fm, const QRect &r, int flags, bool enabled, const QString &text) const override;
    QRect itemPixmapRect(const QRect &r, int flags, const QPixmap &pixmap) const override;
    SubControl hitTestComplexControl(ComplexControl control, const QStyleOptionComplex *option, const QPoint &pos, const QWidget *widget) const override;
    int styleHint(StyleHint hint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *returnData) const override;
    int pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const override;
    int layoutSpacing(QSizePolicy::ControlType control1, QSizePolicy::ControlType control2, Qt::Orientation orientation, const QStyleOption *option, const QWidget *widget) const override;
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
    std::unordered_map<QWidget *, std::unique_ptr<helper::HoverAnimation>> m_buttonAnimation;

    CENTAUR_THEME_INTERFACE_NAMESPACE::ColorScheme *m_colorScheme;
    CENTAUR_THEME_INTERFACE_NAMESPACE::UIElements *m_uiElements;
};

#endif // CENTAUR_CENTTHEME_HPP
