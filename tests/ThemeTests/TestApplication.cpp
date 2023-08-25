/////////////////////////////////////////////////////////////////////////////////////
//
// Created by Ricardo Romero on 12/06/23.
// Copyright (c) 2023 Ricardo Romero.  All rights reserved.
//
#include "TestApplication.hpp"

Window::Window(QWidget *parent) : QMainWindow(parent), m_ui { new Ui::MainWindow }
{
    m_ui->setupUi(this);

    m_ui->lineEdit_4->addAction(QIcon(":/svg/search-gray"), QLineEdit::ActionPosition::LeadingPosition);
    m_ui->checkBox_3->setCheckState(Qt::PartiallyChecked);
    m_ui->checkBox_5->setCheckState(Qt::PartiallyChecked);

    // m_ui->progressBar_5->setValue(50);
    // m_ui->progressBar_5->setFormat("Your text here. " + QString("%1").arg(3) + "%");

    // m_ui->progressBar_6->setRange(0, 0);
}

Window::~Window() = default;
