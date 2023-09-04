/////////////////////////////////////////////////////////////////////////////////////
//
// Created by Ricardo Romero on 02/09/23.
// Copyright (c) 2023 Ricardo Romero.  All rights reserved.
//

#pragma once

#ifndef __cplusplus
#error "C++ compiler needed"
#endif /*__cplusplus*/

#ifndef CENTAUR_TITLEFRAME_HPP
#define CENTAUR_TITLEFRAME_HPP

#include <SystemPushButton.hpp>
#include <WindowFrame.hpp>

BEGIN_CENTAUR_NAMESPACE

class TitleFrame : public WindowFrame
{
    Q_OBJECT
public:
    enum SystemButtons : int
    {
        Close    = 0x00000001,
        Minimize = 0x00000002,
#ifdef Q_OS_MAC
        Fullscreen = 0x00000004,
#else
        Maximize = 0x00000008
#endif /*Q_OS_MAC*/
    };

    explicit TitleFrame(QWidget *parent = nullptr);
    ~TitleFrame() override;

    void setFrameTitle(const QString &title);
    C_NODISCARD QString getFrameTitle() const;

    void setSystemPushButton(int buttons);

    void hideSystemButtons();
    void showSystemButtons();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

    C_P_IMPL()
};

END_CENTAUR_NAMESPACE

#endif // CENTAUR_TITLEFRAME_HPP
