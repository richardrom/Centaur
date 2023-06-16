/////////////////////////////////////////////////////////////////////////////////////
//
// Created by Ricardo Romero on 12/06/23.
// Copyright (c) 2023 Ricardo Romero.  All rights reserved.
//
#include "TestApplication.hpp"

Window::Window(QWidget *parent) :
    m_ui { new Ui::MainWindow }
{
    m_ui->setupUi(this);
}

Window::~Window() = default;
