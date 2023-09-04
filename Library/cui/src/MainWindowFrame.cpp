/////////////////////////////////////////////////////////////////////////////////////
//
// Created by Ricardo Romero on 03/09/23.
// Copyright (c) 2023 Ricardo Romero.  All rights reserved.
//

#include "MainWindowFrame.hpp"
#include "cui.hpp"
#include <QPaintEvent>
#include <QPainterPath>
#include <ThemeInterface.hpp>

BEGIN_CENTAUR_NAMESPACE

struct MainWindowFrame::Impl
{
    const CENTAUR_THEME_INTERFACE_NAMESPACE::MainFrameInformation *uiInformation { nullptr };

    qreal topLeftRadius { 0.0 };
    qreal bottomLeftRadius { 0.0 };
    qreal topRightRadius { 0.0 };
    qreal bottomRightRadius { 0.0 };
};

MainWindowFrame::MainWindowFrame(QWidget *parent) :
    WindowFrame(parent),
    _impl { std::make_unique<MainWindowFrame::Impl>() }
{
}

MainWindowFrame::~MainWindowFrame() = default;

void MainWindowFrame::paintEvent(QPaintEvent *event)
{
    if (cuiTheme() == nullptr) {
        WindowFrame::paintEvent(event);
        return;
    }

    if (P_IMPL()->uiInformation == nullptr) {
        P_IMPL()->uiInformation = std::addressof(cuiTheme()->uiElements().mainFrameInformation);
        assert(P_IMPL()->uiInformation != nullptr);

        const CENTAUR_THEME_INTERFACE_NAMESPACE::FrameInformation &fifo = P_IMPL()->uiInformation->frameInformation;

        const qreal xRadius = fifo.borderRadiusX;
        const qreal yRadius = fifo.borderRadiusY;
        // If xRadius and yRadius are not the same, use the highest of both
        // This simplifies the border drawing
        const auto radius = [&xRadius, &yRadius]() {
            if (!qFuzzyCompare(xRadius, yRadius))
                return std::max(xRadius, yRadius);
            return xRadius;
        }();

        auto _verifyRadius = [&radius](qreal borderRadius) {
            // Verify if the radius is set but not the border left
            // This will set the border radius to either the radius or the border radius, which can be zero
            if (qFuzzyCompare(borderRadius, 0.0) && radius > 0.0)
                return radius;
            return borderRadius;
        };

        P_IMPL()->topLeftRadius     = _verifyRadius(fifo.borderRadiusTopLeft);
        P_IMPL()->topRightRadius    = _verifyRadius(fifo.borderRadiusTopRight);
        P_IMPL()->bottomLeftRadius  = _verifyRadius(fifo.borderRadiusBottomLeft);
        P_IMPL()->bottomRightRadius = _verifyRadius(fifo.borderRadiusBottomRight);
    }

    assert(P_IMPL()->uiInformation != nullptr);

    const auto &topLeftRadius     = P_IMPL()->topLeftRadius;
    const auto &topRightRadius    = P_IMPL()->topRightRadius;
    const auto &bottomLeftRadius  = P_IMPL()->bottomLeftRadius;
    const auto &bottomRightRadius = P_IMPL()->bottomRightRadius;

    static constexpr int g_angle270 = 270;
    static constexpr int g_angle180 = 180;
    static constexpr int g_angle90  = 90;
    static constexpr int g_angle45  = 45;
    static constexpr int g_angle135 = 135;
    static constexpr int g_angle225 = 225;
    static constexpr int g_angle315 = 315;
    static constexpr int g_angle0   = 0;

    QPainter painter(this);
    painter.setRenderHint(QPainter::RenderHint::Antialiasing);
    painter.setBrush(Qt::NoBrush);

    const qreal widgetX = 1;
    const qreal widgetY = 1;
    const qreal width   = geometry().width() - 2;
    const qreal height  = geometry().height() - 2;

    QPainterPath leftBorderPath;
    if (topLeftRadius > 0.0) {
        leftBorderPath.moveTo(
            topLeftRadius,
            widgetY);
        leftBorderPath.arcMoveTo(
            widgetX,
            widgetY,
            topLeftRadius * 2,
            topLeftRadius * 2,
            g_angle135);
        leftBorderPath.arcTo(
            widgetX,
            widgetY,
            topLeftRadius * 2,
            topLeftRadius * 2,
            g_angle135,
            g_angle45);
    }
    else
        leftBorderPath.moveTo(widgetX, widgetY);

    if (bottomLeftRadius > 0.0) {
        leftBorderPath.lineTo(
            widgetX,
            widgetY + height - bottomLeftRadius);
        leftBorderPath.arcTo(widgetX,
            widgetY + height - (bottomLeftRadius * 2),
            bottomLeftRadius * 2,
            bottomLeftRadius * 2,
            g_angle180,
            g_angle45);
    }
    else
        leftBorderPath.lineTo(widgetX, widgetY + height);

    QPainterPath bottomBorderPath;
    if (bottomLeftRadius > 0.0) {
        bottomBorderPath.moveTo(
            widgetX,
            widgetY);
        bottomBorderPath.arcMoveTo(
            widgetX,
            widgetY + height - (bottomLeftRadius * 2),
            bottomLeftRadius * 2,
            bottomLeftRadius * 2,
            g_angle225);
        bottomBorderPath.arcTo(widgetX,
            widgetY + height - (bottomLeftRadius * 2),
            bottomLeftRadius * 2,
            bottomLeftRadius * 2,
            g_angle225,
            g_angle45);
    }
    else
        bottomBorderPath.moveTo(widgetX, widgetY + height);

    if (bottomRightRadius > 0.0) {
        bottomBorderPath.lineTo(
            widgetX + width - bottomRightRadius,
            widgetY + height);
        bottomBorderPath.arcTo(
            widgetX + width - (bottomRightRadius * 2),
            widgetY + height - (bottomRightRadius * 2),
            bottomRightRadius * 2,
            bottomRightRadius * 2,
            g_angle270,
            g_angle45);
    }
    else
        bottomBorderPath.lineTo(widgetX + width, widgetY + height);

    QPainterPath rightBorderPath;

    if (bottomRightRadius > 0.0) {
        rightBorderPath.moveTo(
            widgetX + width - bottomRightRadius,
            widgetY + height - bottomRightRadius);
        rightBorderPath.arcMoveTo(
            widgetX + width - (bottomRightRadius * 2),
            widgetY + height - (bottomRightRadius * 2),
            bottomRightRadius * 2,
            bottomRightRadius * 2,
            g_angle315);
        rightBorderPath.arcTo(
            widgetX + width - (bottomRightRadius * 2),
            widgetY + height - (bottomRightRadius * 2),
            bottomRightRadius * 2,
            bottomRightRadius * 2,
            g_angle315,
            g_angle45);
    }
    else
        rightBorderPath.moveTo(widgetX + width, widgetY + height);

    if (topRightRadius > 0.0) {
        rightBorderPath.lineTo(
            widgetX + width,
            widgetY + topRightRadius);
        rightBorderPath.arcTo(
            widgetX + width - (topRightRadius * 2),
            widgetY,
            topRightRadius * 2,
            topRightRadius * 2,
            g_angle0,
            g_angle45);
    }
    else {
        rightBorderPath.lineTo(
            widgetX + width,
            widgetY);
    }

    QPainterPath topBorderPath;

    if (topRightRadius > 0.0) {
        topBorderPath.moveTo(
            widgetX + width,
            widgetY + topRightRadius);
        topBorderPath.arcMoveTo(
            widgetX + width - (topRightRadius * 2),
            widgetY,
            topRightRadius * 2,
            topRightRadius * 2,
            g_angle45);
        topBorderPath.arcTo(
            widgetX + width - (topRightRadius * 2),
            widgetY,
            topRightRadius * 2,
            topRightRadius * 2,
            g_angle45,
            g_angle45);
    }
    else
        topBorderPath.moveTo(widgetX + width, widgetY);

    if (topLeftRadius > 0.0) {
        topBorderPath.lineTo(
            widgetX + topLeftRadius,
            widgetY);
        topBorderPath.arcTo(
            widgetX,
            widgetY,
            widgetX + (topLeftRadius * 2),
            widgetY + (topLeftRadius * 2),
            g_angle90,
            g_angle45);
    }
    else
        topBorderPath.lineTo(widgetX, widgetY);

    QPainterPath path = topBorderPath;

    if (P_IMPL()->uiInformation->borderPen != Qt::NoPen)
        painter.setPen(Qt::NoPen);

    painter.setBrush(P_IMPL()->uiInformation->backgroundBrush);
    path.connectPath(leftBorderPath);
    path.connectPath(bottomBorderPath);
    path.connectPath(rightBorderPath);
    painter.drawPath(path);

    if (P_IMPL()->uiInformation->frameInformation.topBorderPen != Qt::NoPen)
        painter.setPen(P_IMPL()->uiInformation->frameInformation.topBorderPen);
    painter.drawPath(topBorderPath);

    if (P_IMPL()->uiInformation->frameInformation.leftBorderPen != Qt::NoPen)
        painter.setPen(P_IMPL()->uiInformation->frameInformation.leftBorderPen);
    painter.drawPath(leftBorderPath);

    if (P_IMPL()->uiInformation->frameInformation.bottomBorderPen != Qt::NoPen)
        painter.setPen(P_IMPL()->uiInformation->frameInformation.bottomBorderPen);
    painter.drawPath(bottomBorderPath);

    if (P_IMPL()->uiInformation->frameInformation.rightBorderPen != Qt::NoPen)
        painter.setPen(P_IMPL()->uiInformation->frameInformation.rightBorderPen);
    painter.drawPath(rightBorderPath);
}

END_CENTAUR_NAMESPACE
