/////////////////////////////////////////////////////////////////////////////////////
//
// Created by Ricardo Romero on 13/06/22.
// Copyright (c) 2022 Ricardo Romero.  All rights reserved.
//

#pragma once

#ifndef __cplusplus
#error "C++ compiler needed"
#endif /*__cplusplus*/

#ifndef CENTAUR_ASSETDETAILDIALOG_HPP
#define CENTAUR_ASSETDETAILDIALOG_HPP

#include "BinanceAPI.hpp"
#include "Centaur.hpp"
#include "CentaurInterface.hpp"
#include "OptionsTableWidget.hpp"
#include <QDialog>

#include "../ui/ui_AssetDetailDialog.h"

namespace CENTAUR_NAMESPACE
{
    class AssetDetailDialog : public QDialog
    {
        Q_OBJECT
    public:
        AssetDetailDialog(const BINAPI_NAMESPACE::SPOT::AssetDetail &detail, const QString &asset, CENTAUR_INTERFACE_NAMESPACE::IConfiguration *config, QDialog *parent = nullptr);
        ~AssetDetailDialog() override;

    protected:
        void saveInterfaceState() noexcept;
        void restoreInterfaceState() noexcept;

    private:
        std::unique_ptr<Ui::AssetDetailDialog> m_ui;
    };
} // namespace CENTAUR_NAMESPACE

#endif // CENTAUR_ASSETDETAILDIALOG_HPP
