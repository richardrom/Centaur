/////////////////////////////////////////////////////////////////////////////////////
//
// Created by Ricardo Romero on 04/09/23.
// Copyright (c) 2023 Ricardo Romero.  All rights reserved.
//

#pragma once

#ifndef __cplusplus
#error "C++ compiler needed"
#endif /*__cplusplus*/

#ifndef CENTAUR_SIDEPANELFRAME_HPP
#define CENTAUR_SIDEPANELFRAME_HPP

#include <Centaur.hpp>
#include <QFrame>

BEGIN_CENTAUR_NAMESPACE

class SidePanelFrame : public QFrame
{
    Q_OBJECT
public:
    explicit SidePanelFrame(QWidget *parent = nullptr);
    ~SidePanelFrame() override;

protected:
    void paintEvent(QPaintEvent *event) override;

    C_P_IMPL()
};

END_CENTAUR_NAMESPACE

#endif // CENTAUR_SIDEPANELFRAME_HPP
