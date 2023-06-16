/////////////////////////////////////////////////////////////////////////////////////
//
// Created by Ricardo Romero on 12/06/23.
// Copyright (c) 2023 Ricardo Romero.  All rights reserved.
//

#include "CentTheme.hpp"
#include "QtCore/qpropertyanimation.h"
#include "QtStateMachine/qeventtransition.h"
#include <QDynamicPropertyChangeEvent>
#include <QPushButton>

helper::HoverAnimation::HoverAnimation(
    QObject *parent,
    QWidget *widget,
    const QEasingCurve &easingHover,
    const QEasingCurve &easingLeave,
    const QColor &initialStateColor,
    const QColor &endStateColor,
    int hoverEnterDuration,
    int hoverLeaveDuration) :
    machine { new QStateMachine(parent) },
    stateEnter { new QState },
    stateLeave { new QState }
{
    widget->setProperty("background-color", initialStateColor);

    stateEnter->assignProperty(widget, "background-color", initialStateColor);
    stateLeave->assignProperty(widget, "background-color", endStateColor);

    auto *event1 = new QEventTransition(widget, QEvent::Enter);
    event1->setTargetState(stateLeave);
    event1->addAnimation([widget, parent, hoverEnterDuration, easingHover]() -> QPropertyAnimation * {
        auto property = new QPropertyAnimation(widget, "background-color");
        property->setEasingCurve(easingHover);
        property->setDuration(hoverEnterDuration);

        QObject::connect(property,
            &QPropertyAnimation::valueChanged,
            parent, [widget](C_UNUSED const QVariant &value) {
                widget->repaint();
            });
        return property;
    }());

    stateEnter->addTransition(event1);

    auto *event2 = new QEventTransition(widget, QEvent::Leave);
    event2->setTargetState(stateEnter);
    event2->addAnimation([widget, parent, hoverLeaveDuration, easingLeave]() -> QPropertyAnimation * {
        auto property = new QPropertyAnimation(widget, "background-color");
        property->setEasingCurve(easingLeave);
        property->setDuration(hoverLeaveDuration);

        QObject::connect(property,
            &QPropertyAnimation::valueChanged,
            parent, [widget](C_UNUSED const QVariant &value) {
                widget->repaint();
            });
        return property;
    }());
    stateLeave->addTransition(event2);

    machine->addState(stateEnter);
    machine->addState(stateLeave);

    machine->setInitialState(stateEnter);
    machine->start();
}

helper::HoverAnimation::~HoverAnimation()
{
    delete machine;
}

CentTheme::CentTheme(CENTAUR_THEME_INTERFACE_NAMESPACE::ColorScheme *colorScheme, CENTAUR_THEME_INTERFACE_NAMESPACE::UIElements *uiElements) :
    m_colorScheme { colorScheme },
    m_uiElements { uiElements }
{
}

CentTheme::~CentTheme() = default;

void CentTheme::drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

void CentTheme::drawControl(QStyle::ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    QProxyStyle::drawControl(element, option, painter, widget);
}

void CentTheme::drawComplexControl(QStyle::ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
    QProxyStyle::drawComplexControl(control, option, painter, widget);
}

void CentTheme::drawItemText(QPainter *painter, const QRect &rect, int flags, const QPalette &pal, bool enabled, const QString &text, QPalette::ColorRole textRole) const
{
    QProxyStyle::drawItemText(painter, rect, flags, pal, enabled, text, textRole);
}

void CentTheme::drawItemPixmap(QPainter *painter, const QRect &rect, int alignment, const QPixmap &pixmap) const
{
    QProxyStyle::drawItemPixmap(painter, rect, alignment, pixmap);
}

QSize CentTheme::sizeFromContents(QStyle::ContentsType type, const QStyleOption *option, const QSize &size, const QWidget *widget) const
{
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

QRect CentTheme::subElementRect(QStyle::SubElement element, const QStyleOption *option, const QWidget *widget) const
{
    return QProxyStyle::subElementRect(element, option, widget);
}

QRect CentTheme::subControlRect(QStyle::ComplexControl cc, const QStyleOptionComplex *opt, QStyle::SubControl sc, const QWidget *widget) const
{
    return QProxyStyle::subControlRect(cc, opt, sc, widget);
}

QRect CentTheme::itemTextRect(const QFontMetrics &fm, const QRect &r, int flags, bool enabled, const QString &text) const
{
    return QProxyStyle::itemTextRect(fm, r, flags, enabled, text);
}

QRect CentTheme::itemPixmapRect(const QRect &r, int flags, const QPixmap &pixmap) const
{
    return QProxyStyle::itemPixmapRect(r, flags, pixmap);
}

QStyle::SubControl CentTheme::hitTestComplexControl(QStyle::ComplexControl control, const QStyleOptionComplex *option, const QPoint &pos, const QWidget *widget) const
{
    return QProxyStyle::hitTestComplexControl(control, option, pos, widget);
}

int CentTheme::styleHint(QStyle::StyleHint hint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *returnData) const
{
    return QProxyStyle::styleHint(hint, option, widget, returnData);
}

int CentTheme::pixelMetric(QStyle::PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    return QProxyStyle::pixelMetric(metric, option, widget);
}

int CentTheme::layoutSpacing(QSizePolicy::ControlType control1, QSizePolicy::ControlType control2, Qt::Orientation orientation, const QStyleOption *option, const QWidget *widget) const
{
    return QProxyStyle::layoutSpacing(control1, control2, orientation, option, widget);
}

QIcon CentTheme::standardIcon(QStyle::StandardPixmap standardIcon, const QStyleOption *option, const QWidget *widget) const
{
    return QProxyStyle::standardIcon(standardIcon, option, widget);
}

QPixmap CentTheme::standardPixmap(QStyle::StandardPixmap standardPixmap, const QStyleOption *opt, const QWidget *widget) const
{
    return QProxyStyle::standardPixmap(standardPixmap, opt, widget);
}

QPixmap CentTheme::generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap, const QStyleOption *opt) const
{
    return QProxyStyle::generatedIconPixmap(iconMode, pixmap, opt);
}

QPalette CentTheme::standardPalette() const
{
    return QProxyStyle::standardPalette();
}

void CentTheme::polish(QWidget *widget)
{
    if (auto button = qobject_cast<QPushButton *>(widget);
        button != nullptr)
    {
        const auto &hover = m_uiElements->animations["button-hover-enter"];
        const auto &leave = m_uiElements->animations["button-hover-exit"];
        m_buttonAnimation.insert(
            { widget,
                std::make_unique<helper::HoverAnimation>(
                    this,
                    widget,
                    hover.easingCurve,
                    leave.easingCurve,
                    QColor(0, 0, 0),
                    QColor(255, 128, 4),
                    hover.duration,
                    leave.duration) });
    }

    QProxyStyle::polish(widget);
}

void CentTheme::polish(QPalette &pal)
{
    QProxyStyle::polish(pal);
}

void CentTheme::polish(QApplication *app)
{
    QProxyStyle::polish(app);
}

void CentTheme::unpolish(QWidget *widget)
{
    if (auto btnAnim = m_buttonAnimation.find(widget);
        btnAnim != m_buttonAnimation.end())
    {
        m_buttonAnimation.erase(btnAnim);
    }
    QProxyStyle::unpolish(widget);
}

void CentTheme::unpolish(QApplication *app)
{
    QProxyStyle::unpolish(app);
}
