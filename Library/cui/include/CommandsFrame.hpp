/////////////////////////////////////////////////////////////////////////////////////
//
// Created by Ricardo Romero on 03/09/23.
// Copyright (c) 2023 Ricardo Romero.  All rights reserved.
//

#pragma once

#ifndef __cplusplus
#error "C++ compiler needed"
#endif /*__cplusplus*/

#ifndef CENTAUR_COMMANDSFRAME_HPP
#define CENTAUR_COMMANDSFRAME_HPP

#include <Centaur.hpp>
#include <QFrame>

BEGIN_CENTAUR_NAMESPACE

class CommandsFrame : public QFrame
{
    Q_OBJECT
public:
    explicit CommandsFrame(QWidget *parent = nullptr);
    ~CommandsFrame() override;

protected:
    void paintEvent(QPaintEvent *event) override;

    C_P_IMPL()
};

END_CENTAUR_NAMESPACE

#endif // CENTAUR_COMMANDSFRAME_HPP
