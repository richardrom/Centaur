/////////////////////////////////////////////////////////////////////////////////////
//
// Created by Ricardo Romero on 03/09/23.
// Copyright (c) 2023 Ricardo Romero.  All rights reserved.
//

#pragma once

#ifndef __cplusplus
#error "C++ compiler needed"
#endif /*__cplusplus*/

#ifndef CENTAUR_MAINWINDOWFRAME_HPP
#define CENTAUR_MAINWINDOWFRAME_HPP

#include <WindowFrame.hpp>
BEGIN_CENTAUR_NAMESPACE

/// \brief This Frame draws the background for the main window frame
class MainWindowFrame : public WindowFrame
{
    Q_OBJECT
public:
    explicit MainWindowFrame(QWidget *parent = nullptr);
    ~MainWindowFrame() override;

protected:
    void paintEvent(QPaintEvent *event) override;

    C_P_IMPL()
};

END_CENTAUR_NAMESPACE

#endif // CENTAUR_MAINWINDOWFRAME_HPP
