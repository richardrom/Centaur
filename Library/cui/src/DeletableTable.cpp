/////////////////////////////////////////////////////////////////////////////////////
//
// Created by Ricardo Romero on 10/04/23.
// Copyright (c) 2023 Ricardo Romero.  All rights reserved.
//

#include "DeletableTable.hpp"

#include <QKeyEvent>

cen::DeletableTable::DeletableTable(QWidget *parent) :
    QTableView(parent)
{
}

void cen::DeletableTable::keyPressEvent(QKeyEvent *event)
{
    const int key = event->key();
    if (!event->modifiers() && (key == Qt::Key_Backspace || key == Qt::Key_Delete))
    {
        emit deleteKeyPressed();
    }
}
