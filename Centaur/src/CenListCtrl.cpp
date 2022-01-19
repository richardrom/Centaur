/////////////////////////////////////////////////////////////////////////////////////
//
// Created by Ricardo Romero on 06/12/21.
// Copyright (c) 2021 Ricardo Romero.  All rights reserved.
//

#include "CenListCtrl.hpp"
#include <QMenu>
#include <QMouseEvent>

cent::CenListCtrl::CenListCtrl(QWidget *parent) :
    QTableView(parent)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &cent::CenListCtrl::customContextMenuRequested, this, &cent::CenListCtrl::showContextMenu);
}

void cent::CenListCtrl::setRemove(bool status)
{
    m_remove = status;
}

void cent::CenListCtrl::allowClickMessages(bool status)
{
    m_allowClickMessages = status;
}

void cent::CenListCtrl::showContextMenu(const QPoint &pos)
{
    QModelIndex index = indexAt(pos).siblingAtColumn(0);
    auto itemData     = index.data(Qt::DisplayRole).toString();

    if (!itemData.isEmpty())
    {
        QMenu contextMenu("Context menu", this);

        QAction action(
            !m_remove
                ? QString(tr("Add '%1' to the watchlist")).arg(itemData)
                : QString(tr("Remove '%1' from the watchlist")).arg(itemData),
            this);

        if (!m_remove)
        {
            connect(&action, &QAction::triggered, this,
                [&]() { emit sgAddToWatchList(itemData, objectName()); });
        }
        else
        {
            connect(&action, &QAction::triggered, this,
                [&]() { emit sgRemoveWatchList(index.row()); });
        }

        contextMenu.addAction(&action);
        contextMenu.exec(mapToGlobal(pos));
        /*if (!m_remove)
{
    QAction action(QString(tr("Add '%1' to the watchlist")).arg(itemData), this);
    connect(&action, &QAction::triggered, this,
        [&]() { emit sgAddToWatchList(itemData, objectName()); });
    contextMenu.addAction(&action);
}
else
{
    QAction action0(QString(tr("Add '%1' to the watchlist")).arg(itemData), this);
    connect(&action0, &QAction::triggered, this,
        [&]() { emit sgRemoveWatchList(index.row()); });

QAction action1(QString(tr("Add '%1' to favorites")).arg(itemData), this);
connect(&action1, &QAction::triggered, this,
    [&]() { emit sgRemoveWatchList(index.row()); });

contextMenu.addAction(&action0);
contextMenu.addSeparator();
contextMenu.addAction(&action1);
}*/
    }
}

void cent::CenListCtrl::mousePressEvent(QMouseEvent *event)
{
    QTableView::mousePressEvent(event);
    if (!m_allowClickMessages)
        return;

    auto itemModel = indexAt(event->pos());

    if (itemModel.isValid())
    {
        QString source = itemModel.siblingAtColumn(4).data().toString();
        QString symbol = itemModel.siblingAtColumn(0).data().toString();
        emit sgSetSelection(source, symbol);
    }
    else
    {
        emit sgRemoveSelection();
    }
}
