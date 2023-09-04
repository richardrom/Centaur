////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Created by Ricardo Romero on 15/04/22.
//  Copyright (c) 2022 Ricardo Romero.  All rights reserved.
//

#include "SystemPushButton.hpp"

#include <QApplication>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>

BEGIN_CENTAUR_NAMESPACE

struct SystemPushButton::Impl
{
    SystemPushButton::ButtonClass buttonClass { SystemPushButton::ButtonClass::Close };
    bool mouseInside { false };
};

SystemPushButton::SystemPushButton(QWidget *parent) :
    QFrame(parent),
    _impl { std::make_unique<SystemPushButton::Impl>() }
{
    setFrameShape(QFrame::NoFrame);
    setLineWidth(0);
    setFocusPolicy(Qt::FocusPolicy::NoFocus);
    setMouseTracking(true);
}

SystemPushButton::SystemPushButton(SystemPushButton::ButtonClass buttonClass, QWidget *parent) :
    SystemPushButton(parent)
{
    P_IMPL()->buttonClass = buttonClass;
}

SystemPushButton::~SystemPushButton() = default;

void SystemPushButton::setButtonClass(CENTAUR_NAMESPACE::SystemPushButton::ButtonClass btnClass) noexcept
{
    P_IMPL()->buttonClass = btnClass;
    update();
}

void SystemPushButton::mousePressEvent(C_UNUSED QMouseEvent *event)
{
}

void SystemPushButton::mouseReleaseEvent(C_UNUSED QMouseEvent *event)
{
    emit buttonPressed();
}

void SystemPushButton::enterEvent(C_UNUSED QEnterEvent *event)
{
    P_IMPL()->mouseInside = true;
    update();
}

void SystemPushButton::leaveEvent(C_UNUSED QEvent *event)
{
    P_IMPL()->mouseInside = false;
    update();
}

void SystemPushButton::paintEvent(C_UNUSED QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::RenderHint::Antialiasing);
#if defined(Q_OS_MAC)

    const QBrush brush = [&]() -> QBrush {
        switch (P_IMPL()->buttonClass) {
            case ButtonClass::Close:
                {
                    static constexpr auto closePushButtonColor       = QColor(245, 79, 83);
                    static constexpr auto closePushButtonColor_Hover = QColor(246, 95, 87);
                    if (P_IMPL()->mouseInside) {
                        return QBrush { closePushButtonColor_Hover };
                    }
                    return QBrush { closePushButtonColor };
                }
            case ButtonClass::Minimize:
                {
                    static constexpr auto minimizePushButtonColor       = QColor(250, 178, 67);
                    static constexpr auto minimizePushButtonColor_Hover = QColor(250, 188, 47);
                    if (P_IMPL()->mouseInside) {
                        return QBrush { minimizePushButtonColor_Hover };
                    }
                    return QBrush { minimizePushButtonColor };
                }
            case ButtonClass::Fullscreen:
                {
                    static constexpr auto fullscreenPushButtonColor       = QColor(68, 200, 64);
                    static constexpr auto fullscreenPushButtonColor_Hover = QColor(87, 205, 83);
                    if (P_IMPL()->mouseInside) {
                        return QBrush { fullscreenPushButtonColor_Hover };
                    }
                    return QBrush { fullscreenPushButtonColor };
                }
        }
    }();

    painter.setPen(Qt::NoPen);
    painter.setBrush(brush);
    painter.drawRoundedRect(
        event->rect(),
        static_cast<qreal>(event->rect().width()) / 2,
        static_cast<qreal>(event->rect().height()) / 2);

#elif defined(Q_OS_WIN)
    // TODO: Draw windows widgets
#elif defined(Q_OS_LINUX)
    // TODO: Draw linux widgets
#endif
}

END_CENTAUR_NAMESPACE
