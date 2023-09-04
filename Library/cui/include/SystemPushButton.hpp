////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Created by Ricardo Romero on 15/04/22.
//  Copyright (c) 2022 Ricardo Romero.  All rights reserved.
//

#pragma once

#ifndef __cplusplus
#error "C++ compiler needed"
#endif /*__cplusplus*/

#ifndef CENTAUR_SYSTEMPUSHBUTTON_HPP
#define CENTAUR_SYSTEMPUSHBUTTON_HPP

#include "Centaur.hpp"
#include <QFrame>

BEGIN_CENTAUR_NAMESPACE

/// \brief The TitleFrame class initializes all SystemPushButtons
/// And it is not intended to use independently
class SystemPushButton : public QFrame
{
    Q_OBJECT
public:
    enum class ButtonClass
    {
        Close = 0,
        Minimize,
#ifdef Q_OS_MAC
        Fullscreen,
#else
        Maximize,
#endif /* Q_OS_MAC */
    };
public:
    explicit SystemPushButton(QWidget *parent = nullptr);
    explicit SystemPushButton(ButtonClass buttonClass, QWidget *parent = nullptr);
    ~SystemPushButton() override;

    void setButtonClass(ButtonClass btnClass) noexcept;

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

signals:
    /// \brief This signal is emitted when the close button is pressed if
    void buttonPressed();

    C_P_IMPL()
};

END_CENTAUR_NAMESPACE

#endif // CENTAUR_SYSTEMPUSHBUTTON_HPP
