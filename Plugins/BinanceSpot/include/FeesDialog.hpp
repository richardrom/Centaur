/////////////////////////////////////////////////////////////////////////////////////
//
// Created by Ricardo Romero on 13/06/22.
// Copyright (c) 2022 Ricardo Romero.  All rights reserved.
//

#pragma once

#ifndef __cplusplus
#error "C++ compiler needed"
#endif /*__cplusplus*/

#ifndef CENTAUR_FEESDIALOG_HPP
#define CENTAUR_FEESDIALOG_HPP

#include "BinanceAPI.hpp"
#include "Centaur.hpp"
#include "CentaurInterface.hpp"
#include "OptionsTableWidget.hpp"
#include <QDialog>

#include "../ui/ui_FeesDialog.h"

namespace CENTAUR_NAMESPACE
{
    class TradeFeesDialog : public QDialog
    {
        Q_OBJECT
    public:
        TradeFeesDialog(const BINAPI_NAMESPACE::SpotTradingFees &fees, const QString &symbolFilter, QDialog *parent = nullptr);
        ~TradeFeesDialog() override;

    protected:
        void saveInterfaceState() noexcept;
        void restoreInterfaceState() noexcept;

    private:
        QStandardItemModel *m_searchModel { nullptr };

    private:
        std::unique_ptr<Ui::FeesDialog> m_ui;
    };
} // namespace CENTAUR_NAMESPACE

#endif // CENTAUR_FEESDIALOG_HPP
