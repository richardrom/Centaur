/////////////////////////////////////////////////////////////////////////////////////
//
// Created by Ricardo Romero on 12/06/23.
// Copyright (c) 2023 Ricardo Romero.  All rights reserved.
//

#pragma once

#ifndef __cplusplus
#error "C++ compiler needed"
#endif /*__cplusplus*/

#ifndef CENTAUR_TESTAPPLICATION_HPP
#define CENTAUR_TESTAPPLICATION_HPP

#include "../ui_window.h"
#include <QMainWindow>

class Window : public QMainWindow
{
public:
    Window(QWidget *parent = nullptr);
    ~Window() override;

private:
    std::unique_ptr<Ui::MainWindow> m_ui;
};

#endif // CENTAUR_TESTAPPLICATION_HPP
