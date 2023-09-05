/////////////////////////////////////////////////////////////////////////////////////
//
// Created by Ricardo Romero on 02/09/23.
// Copyright (c) 2023 Ricardo Romero.  All rights reserved.
//
#include "TitleFrame.hpp"
#include "cui.hpp"
#include <QApplication>
#include <QFontMetrics>
#include <QPaintEvent>
#include <QPainterPath>
#include <QResizeEvent>
#include <ThemeInterface.hpp>

BEGIN_CENTAUR_NAMESPACE

static auto initFontFromInfo(const QFont &font_p, const cen::theme::FontTextLayout &fifo) noexcept -> QFont
{
    QFont font = font_p;

    font.setCapitalization(fifo.style.caps);
    if (!fifo.style.fontName.isEmpty())
        font.setFamily(fifo.style.fontName);

    font.setItalic(fifo.style.italic);
    font.setKerning(fifo.style.kerning);
    font.setUnderline(fifo.style.underline);

    if (fifo.style.size > 0)
        font.setPointSizeF(fifo.style.size);

    font.setWeight(static_cast<QFont::Weight>(fifo.style.weight));

    if (fifo.style.wordSpacing > -1)
        font.setWordSpacing(fifo.style.wordSpacing);

    font.setStretch(fifo.style.stretchFactor);

    if (fifo.style.letterSpacing > -1)
        font.setLetterSpacing(fifo.style.spacingType, fifo.style.letterSpacing);

    return font;
}

struct TitleFrame::Impl
{
    void initData(QWidget *titleWidget);

    QString frameTitle;

    const CENTAUR_THEME_INTERFACE_NAMESPACE::TitleBarInformation *uiInformation { nullptr };

    qreal topLeftRadius { 0.0 };
    qreal bottomLeftRadius { 0.0 };
    qreal topRightRadius { 0.0 };
    qreal bottomRightRadius { 0.0 };

    SystemPushButton *closeButton { nullptr };
    SystemPushButton *minimizeButton { nullptr };
    SystemPushButton *maximizeButton { nullptr }; // Under macOS, this is the fullscreen button

    bool previousFullScreenStatusWasMax = false;
};

void TitleFrame::Impl::initData(QWidget *titleWidget)
{
    if (uiInformation == nullptr) {
        const auto widgetObjectName = titleWidget->objectName();
        const auto iter             = cuiTheme()->uiElements().titleBarOverride.find(widgetObjectName);
        if (iter != cuiTheme()->uiElements().titleBarOverride.end()) {
            uiInformation = std::addressof(iter->second);
        }
        else
            uiInformation = std::addressof(cuiTheme()->uiElements().titleBarInformation);

        const CENTAUR_THEME_INTERFACE_NAMESPACE::FrameInformation &fifo = uiInformation->frameInformation;

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

        topLeftRadius     = _verifyRadius(fifo.borderRadiusTopLeft);
        topRightRadius    = _verifyRadius(fifo.borderRadiusTopRight);
        bottomLeftRadius  = _verifyRadius(fifo.borderRadiusBottomLeft);
        bottomRightRadius = _verifyRadius(fifo.borderRadiusBottomRight);
    }
}

TitleFrame::TitleFrame(QWidget *parent) :
    WindowFrame(WindowFrame::FrameMode::Movable, parent),
    _impl { std::make_unique<TitleFrame::Impl>() }
{
    // Usually, this frame will be contained inside a main WindowFrame,
    // which is, usually, within a QDialog or a CDialog or a main window
    // in either case, set the movable parent to this Widget
    const auto *parentClassName = parent->metaObject()->className();
    if (strcmp(parentClassName, "cen::WindowFrame") == 0 && parent->parent() != nullptr) {
        const auto *parentParentClassName = parent->parent()->metaObject()->className();
        if (strcmp(parentParentClassName, "cen::CDialog") == 0
            || strcmp(parentParentClassName, "QDialog") == 0) {
            overrideMovableParent(parent->parentWidget());
            // On Dialogs, set only the close button by default
            setSystemPushButton(TitleFrame::SystemButtons::Close);
        }
    }
    else if (strcmp(parentClassName, "cen::MainWindowFrame") == 0) {
        // Override to the top most QWidget
        overrideMovableParent();
        setSystemPushButton(
            TitleFrame::SystemButtons::Close
            | TitleFrame::SystemButtons::Minimize
#ifdef Q_OS_MAC
            | TitleFrame::SystemButtons::Fullscreen
#else
            | TitleFrame::SystemButtons::Maximize
#endif /*Q_OS_MAC*/
        );
    }
}

TitleFrame::~TitleFrame() = default;

void TitleFrame::paintEvent(QPaintEvent *event)
{
    if (cuiTheme() == nullptr) {
        WindowFrame::paintEvent(event);
        return;
    }

    P_IMPL()->initData(this);

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

    const qreal widgetX = 0;
    const qreal widgetY = 0;
    const qreal width   = event->rect().width();
    const qreal height  = event->rect().height();

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

    painter.setPen(Qt::NoPen);
    painter.setBrush(P_IMPL()->uiInformation->backgroundBrush);
    path.connectPath(leftBorderPath);
    path.connectPath(bottomBorderPath);
    path.connectPath(rightBorderPath);
    painter.drawPath(path);

    if (!P_IMPL()->frameTitle.isEmpty()) {
        painter.setPen(P_IMPL()->uiInformation->fontPen);
        painter.setFont(initFontFromInfo(QApplication::font(), P_IMPL()->uiInformation->font));
        const QFontMetrics fontMetrics(initFontFromInfo(QApplication::font(), P_IMPL()->uiInformation->font));
        const QString elidedText = fontMetrics.elidedText(P_IMPL()->frameTitle, Qt::TextElideMode::ElideRight, static_cast<int>(width), 0);
        painter.drawText(event->rect(), elidedText, P_IMPL()->uiInformation->font.opts);
    }
}

void TitleFrame::setFrameTitle(const QString &title)
{
    P_IMPL()->initData(this);
    P_IMPL()->frameTitle = title;
    update();
}

QString TitleFrame::getFrameTitle() const
{
    return P_IMPL()->frameTitle;
}

void TitleFrame::hideSystemButtons()
{
    if (P_IMPL()->closeButton)
        P_IMPL()->closeButton->hide();
    if (P_IMPL()->minimizeButton)
        P_IMPL()->minimizeButton->hide();
    if (P_IMPL()->maximizeButton)
        P_IMPL()->maximizeButton->hide();
}

void TitleFrame::showSystemButtons()
{
    if (P_IMPL()->closeButton)
        P_IMPL()->closeButton->show();
    if (P_IMPL()->minimizeButton)
        P_IMPL()->minimizeButton->show();
    if (P_IMPL()->maximizeButton)
        P_IMPL()->maximizeButton->show();
}

void TitleFrame::setSystemPushButton(int buttons)
{
    delete P_IMPL()->maximizeButton;
    delete P_IMPL()->minimizeButton;
    delete P_IMPL()->closeButton;

    const bool parentIsDialog = [&]() -> bool {
        if (activeParent() != nullptr) {
            const auto *parentClassName = activeParent()->metaObject()->className();
            if (strcmp(parentClassName, "cen::CDialog") == 0
                || strcmp(parentClassName, "QDialog") == 0) {
                return true;
            }
        }
        return false;
    }();

    if (buttons & TitleFrame::SystemButtons::Close) {

        P_IMPL()->closeButton = new SystemPushButton(SystemPushButton::ButtonClass::Close, this);

        if (parentIsDialog) {
            connect(P_IMPL()->closeButton,
                &SystemPushButton::buttonPressed,
                qobject_cast<QDialog *>(activeParent()),
                &QDialog::reject);
        }
        else {
            connect(P_IMPL()->closeButton,
                &SystemPushButton::buttonPressed,
                activeParent(),
                [activeParent = activeParent()] {
                    activeParent->close();
                });
        }
    }

    if (buttons & TitleFrame::SystemButtons::Minimize) {
        P_IMPL()->minimizeButton = new SystemPushButton(SystemPushButton::ButtonClass::Minimize, this);
        connect(P_IMPL()->minimizeButton,
            &SystemPushButton::buttonPressed,
            activeParent(),
            [parent = activeParent()]() {
                parent->showMinimized();
            });
    }

#ifdef Q_OS_MAC
    if (buttons & TitleFrame::SystemButtons::Fullscreen) {
        P_IMPL()->maximizeButton = new SystemPushButton(SystemPushButton::ButtonClass::Fullscreen, this);
        connect(P_IMPL()->maximizeButton,
            &SystemPushButton::buttonPressed,
            activeParent(),
            [parent             = activeParent(),
                &prevStatus     = P_IMPL()->previousFullScreenStatusWasMax,
                &minimizeButton = P_IMPL()->minimizeButton] {
                if (!parent->isFullScreen()) {
                    prevStatus = parent->isMaximized();
                    parent->showFullScreen();
                    minimizeButton->setEnabled(false);
                }
                else {
                    minimizeButton->setEnabled(true);
                    if (prevStatus) {
                        parent->showMaximized();
                    }
                    else
                        parent->showNormal();
                }
            });
    }
#else
    if (buttons & TitleFrame::SystemButtons::Maximize)
        P_IMPL()->maximizeButton = new SystemPushButton(this);
        // TODO: Maximize button
#endif /*Q_OS_MAC*/

    showSystemButtons();
}

void TitleFrame::resizeEvent(QResizeEvent *event)
{
    const auto &size = event->size();
#ifdef Q_OS_MAC
    static constexpr int buttonSpacing = 8;
    static constexpr int maxSize       = 12;
    static constexpr int xPosition     = 10;
    const int yPosition                = (size.height() - maxSize) / 2;
#else
    // TODO: DO WINDOWS AND LINUX
#endif
    int xAdvance = xPosition;
    if (P_IMPL()->closeButton) {
        P_IMPL()->closeButton->setGeometry(xAdvance, yPosition, maxSize, maxSize);
        xAdvance += maxSize + buttonSpacing;
    }

    if (P_IMPL()->minimizeButton) {
        P_IMPL()->minimizeButton->setGeometry(xAdvance, yPosition, maxSize, maxSize);
        xAdvance += maxSize + buttonSpacing;
    }

    if (P_IMPL()->maximizeButton) {
        P_IMPL()->maximizeButton->setGeometry(xAdvance, yPosition, maxSize, maxSize);
    }

    WindowFrame::resizeEvent(event);
}

END_CENTAUR_NAMESPACE
