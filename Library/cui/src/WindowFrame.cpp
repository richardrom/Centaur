/////////////////////////////////////////////////////////////////////////////////////
//
// Created by Ricardo Romero on 29/08/23.
// Copyright (c) 2023 Ricardo Romero.  All rights reserved.
//

#include "WindowFrame.hpp"
#include <QMouseEvent>
#include <QPainter>

BEGIN_CENTAUR_NAMESPACE

enum HotSpots : int
{
    topLeftCorner = 0,
    topRightCorner,
    bottomLeftCorner,
    bottomRightCorner,
    topBorder,
    bottomBorder,
    rightBorder,
    leftBorder
};

struct WindowFrame::Impl
{
    static constexpr auto maxActiveFrames = 8ULL;

    Impl(WindowFrame::FrameMode frameMode, QWidget *parent) :
        mode { frameMode },
        topLevelParent { parent } { }

    auto isPointInActiveFrame(const QPoint &point) -> int
    {
        auto idx = 0UL;
        while (idx < activeFrames.size()) {
            if (activeFrames.at(idx).contains(point)) {
                return static_cast<int>(idx);
            }
            ++idx;
        }
        return -1;
    }

    WindowFrame::FrameMode mode;

    QWidget *topLevelParent { nullptr };
    QPoint startPoint;

    // Resizeable parameters
    int mouseAt { -1 };
    int bottomDiff { 0 };
    int rightDiff { 0 };
    int leftDiff { 0 };
    int topDiff { 0 };
    std::array<QRect, maxActiveFrames> activeFrames;

    // Movable parameters
    QRect normalGeometry;
    bool thisEvent { false };
};

WindowFrame::WindowFrame(QWidget *parent) noexcept :
    QFrame(parent),
    _impl { std::make_unique<Impl>(WindowFrame::FrameMode::AllModes, parent) }
{
    setMouseTracking(true);
}

WindowFrame::WindowFrame(WindowFrame::FrameMode mode, QWidget *parent) noexcept :
    QFrame(parent),
    _impl { std::make_unique<Impl>(mode, parent) }
{
    setMouseTracking(true);
}

WindowFrame::~WindowFrame() = default;

auto WindowFrame::overrideMovableParent(QWidget *widget) -> void
{
    assert(widget != nullptr);
    P_IMPL()->topLevelParent = widget;
}

auto WindowFrame::overrideMovableParent() -> QWidget *
{
    while (activeParent()->parentWidget() != nullptr)
        P_IMPL()->topLevelParent = activeParent()->parentWidget();
    return activeParent();
}

QWidget *WindowFrame::activeParent() const
{
    assert(P_IMPL()->topLevelParent != nullptr);
    return P_IMPL()->topLevelParent;
}

void WindowFrame::leaveEvent(C_UNUSED QEvent *event)
{
    setCursor(Qt::ArrowCursor);
    update();
}

void WindowFrame::mousePressEvent(QMouseEvent *event)
{
    const QPoint localPoint = event->pos();

    bool resizeActive = false;
    if (P_IMPL()->mode == WindowFrame::FrameMode::Resizable
        || P_IMPL()->mode == WindowFrame::FrameMode::AllModes) {

        P_IMPL()->mouseAt = P_IMPL()->isPointInActiveFrame(localPoint);
        if (P_IMPL()->mouseAt >= 0) {
            resizeActive = true;

            P_IMPL()->bottomDiff = activeParent()->geometry().bottom() - event->globalPosition().toPoint().y();
            P_IMPL()->rightDiff  = activeParent()->geometry().right() - event->globalPosition().toPoint().x();
            P_IMPL()->leftDiff   = event->globalPosition().toPoint().x() - activeParent()->geometry().left();
            P_IMPL()->topDiff    = event->globalPosition().toPoint().y() - activeParent()->geometry().top();
        }
    }

    bool movableActive = false;
    if ((P_IMPL()->mode == WindowFrame::FrameMode::Movable
            || P_IMPL()->mode == WindowFrame::FrameMode::AllModes)
        && !resizeActive) {

        if (event->buttons().testFlag(Qt::LeftButton)) {
            movableActive = true;
            // There are some widgets that propagate the mouseMoveEvent to this Widget
            // and start moving the whole window or dialog without a direct mouse press on this WindowFrame (QComboBox, QToolButton),
            // Taking advantage of this; with the flag, m_thisEvent, mouseMoveEvent can be ignored unless
            // the user presses the MovableFrame directly

            P_IMPL()->thisEvent = true;
        }
    }

    // We'll grab the mouse in either situation
    if (movableActive || resizeActive) {
        P_IMPL()->startPoint = event->globalPosition().toPoint();
        grabMouse();
    }
}

void WindowFrame::mouseReleaseEvent(QMouseEvent *event)
{
    P_IMPL()->thisEvent = false;
    P_IMPL()->mouseAt   = -1;
    releaseMouse();

    QWidget::mouseReleaseEvent(event);
}

void WindowFrame::mouseMoveEvent(QMouseEvent *event)
{
    if (P_IMPL()->mode == WindowFrame::FrameMode::Resizable
        || P_IMPL()->mode == WindowFrame::FrameMode::AllModes) {
        const QPoint localPoint = event->pos();
        QRect geometry          = activeParent()->geometry();

        auto cursor       = Qt::ArrowCursor;
        P_IMPL()->mouseAt = P_IMPL()->isPointInActiveFrame(localPoint);

        switch (P_IMPL()->mouseAt) {
            case HotSpots::topLeftCorner: C_FALLTHROUGH;
            case HotSpots::bottomRightCorner: cursor = Qt::SizeFDiagCursor; break;
            case HotSpots::topRightCorner: C_FALLTHROUGH;
            case HotSpots::bottomLeftCorner: cursor = Qt::SizeBDiagCursor; break;
            case HotSpots::topBorder: C_FALLTHROUGH;
            case HotSpots::bottomBorder: cursor = Qt::SizeVerCursor; break;
            case HotSpots::rightBorder: C_FALLTHROUGH;
            case HotSpots::leftBorder: cursor = Qt::SizeHorCursor; break;
            default: cursor = Qt::ArrowCursor;
        }

        setCursor(cursor);
        update();

        if (event->buttons().testFlag(Qt::LeftButton)) {
            const QPoint globalPoint = event->globalPosition().toPoint();

            if (const QPoint diff = P_IMPL()->startPoint - globalPoint; diff != QPoint(0, 0)) {
                switch (P_IMPL()->mouseAt) {
                    case HotSpots::topLeftCorner: geometry.setTopLeft(globalPoint - QPoint { P_IMPL()->leftDiff, P_IMPL()->topDiff }); break;
                    case HotSpots::topRightCorner: geometry.setTopRight(globalPoint + QPoint { P_IMPL()->rightDiff, -P_IMPL()->topDiff }); break;
                    case HotSpots::bottomLeftCorner: geometry.setBottomLeft(globalPoint + QPoint { -P_IMPL()->leftDiff, P_IMPL()->bottomDiff }); break;
                    case HotSpots::bottomRightCorner: geometry.setBottomRight(globalPoint + QPoint { P_IMPL()->rightDiff, P_IMPL()->bottomDiff }); break;
                    case HotSpots::topBorder: geometry.setTop(globalPoint.y() - P_IMPL()->topDiff); break;
                    case HotSpots::bottomBorder: geometry.setBottom(globalPoint.y() + P_IMPL()->bottomDiff); break;
                    case HotSpots::rightBorder: geometry.setRight(globalPoint.x() + P_IMPL()->rightDiff); break;
                    case HotSpots::leftBorder: geometry.setLeft(globalPoint.x() - P_IMPL()->leftDiff); break;
                }
            }
            P_IMPL()->startPoint = globalPoint;
            activeParent()->setGeometry(geometry);
        }
    }
    else if (P_IMPL()->mode == WindowFrame::FrameMode::Movable
             || P_IMPL()->mode == WindowFrame::FrameMode::AllModes) {

        if (event->buttons().testFlag(Qt::LeftButton) && P_IMPL()->thisEvent) {
            if ((event->globalPosition().toPoint() - P_IMPL()->startPoint) != QPoint { 0, 0 }) {
                QRect thisRect = activeParent()->geometry();
                thisRect.moveTopLeft(thisRect.topLeft() + (event->globalPosition().toPoint() - P_IMPL()->startPoint));
                activeParent()->setGeometry(thisRect);
            }

            P_IMPL()->startPoint = event->globalPosition().toPoint();
        }
    }
}

void WindowFrame::mouseDoubleClickEvent(C_UNUSED QMouseEvent *event)
{
    if (P_IMPL()->mode == WindowFrame::FrameMode::Resizable)
        return;

    if (activeParent()->isMaximized()) {
        activeParent()->showNormal();
        if (!P_IMPL()->normalGeometry.isEmpty())
            activeParent()->setGeometry(P_IMPL()->normalGeometry);
    }
    else {
        P_IMPL()->normalGeometry = activeParent()->geometry();
        activeParent()->showMaximized();
    }
}

void WindowFrame::resizeEvent(QResizeEvent *event)
{
    static constexpr int frameDim        = 5;
    static constexpr int borderDim       = 3;
    static constexpr int totalCornersDim = frameDim * 2;

    P_IMPL()->activeFrames[topLeftCorner] = {
        0, 0,
        frameDim,
        frameDim
    };

    P_IMPL()->activeFrames[topRightCorner] = {
        event->size().width() - frameDim,
        0,
        frameDim,
        frameDim
    };

    P_IMPL()->activeFrames[bottomLeftCorner] = {
        0,
        event->size().height() - frameDim,
        frameDim,
        frameDim
    };

    P_IMPL()->activeFrames[bottomRightCorner] = {
        event->size().width() - frameDim,
        event->size().height() - frameDim,
        frameDim,
        frameDim
    };

    P_IMPL()->activeFrames[topBorder] = {
        frameDim,
        0,
        event->size().width() - totalCornersDim,
        borderDim
    };

    P_IMPL()->activeFrames[rightBorder] = {
        event->size().width() - borderDim,
        frameDim, borderDim,
        event->size().height() - totalCornersDim
    };

    P_IMPL()->activeFrames[leftBorder] = {
        0,
        frameDim,
        borderDim,
        event->size().height() - totalCornersDim
    };

    P_IMPL()->activeFrames[bottomBorder] = {
        frameDim,
        event->size().height() - borderDim,
        event->size().width() - totalCornersDim,
        borderDim
    };

    QWidget::resizeEvent(event);
}

void WindowFrame::paintEvent(QPaintEvent *event)
{
    QFrame::paintEvent(event); /*
     QPainter painter(this);
     painter.setBrush(QBrush(QColor(0, 255, 255)));
     for (const auto &frame : P_IMPL()->activeFrames) {
         painter.drawRect(frame);
     }*/
}

WindowFrame::FrameMode WindowFrame::frameMode() const
{
    return _impl->mode;
}

END_CENTAUR_NAMESPACE
