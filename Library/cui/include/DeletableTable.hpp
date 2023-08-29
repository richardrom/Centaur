/////////////////////////////////////////////////////////////////////////////////////
//
// Created by Ricardo Romero on 10/04/23.
// Copyright (c) 2023 Ricardo Romero.  All rights reserved.
//

#pragma once

#ifndef __cplusplus
#error "C++ compiler needed"
#endif /*__cplusplus*/

#ifndef CENTAUR_DELETABLETABLE_HPP
#define CENTAUR_DELETABLETABLE_HPP

#include "Centaur.hpp"

#include <QTableView>

BEGIN_CENTAUR_NAMESPACE

class DeletableTable : public QTableView
{
    Q_OBJECT
public:
    explicit DeletableTable(QWidget *parent = nullptr);
    ~DeletableTable() override = default;

signals:
    void deleteKeyPressed();

protected:
    void keyPressEvent(QKeyEvent *event) override;
};

END_CENTAUR_NAMESPACE

#endif // CENTAUR_DELETABLETABLE_HPP
