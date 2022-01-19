/////////////////////////////////////////////////////////////////////////////////////
//
// Created by Ricardo Romero on 06/12/21.
// Copyright (c) 2021 Ricardo Romero.  All rights reserved.
//

#pragma once

#ifndef CENTAUR_CENLISTCTRL_HPP
#define CENTAUR_CENLISTCTRL_HPP
#include <QTableView>

namespace cent
{
    class CenListCtrl : public QTableView
    {
        Q_OBJECT
    public:
        explicit CenListCtrl(QWidget *parent = nullptr);

    public:
        void setRemove(bool status = true);
        void allowClickMessages(bool status = true);

    protected:
        void showContextMenu(const QPoint &pos);

    signals:
        void sgAddToWatchList(const QString &symbol, const QString &listName);
        void sgRemoveWatchList(const int &row);
        void sgSetSelection(const QString &source, const QString &symbol);
        void sgRemoveSelection();

    private:
        bool m_remove { false };
        bool m_allowClickMessages { false };

        // QWidget interface
    protected:
        void mousePressEvent(QMouseEvent *event) override;
    };
} // namespace cent

#endif // CENTAUR_CENLISTCTRL_HPP
