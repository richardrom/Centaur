/////////////////////////////////////////////////////////////////////////////////////
//
// Created by Ricardo Romero on 27/02/23.
// Copyright (c) 2023 Ricardo Romero.  All rights reserved.
//

#ifndef CENTAUR_SETTINGSWIDGET_HPP
#define CENTAUR_SETTINGSWIDGET_HPP

#include <QStandardItemModel>
#include <QTableView>
#include <QWidget>

#include <CentaurPlugin.hpp>

#include "ExchangeRate.hpp"

namespace cen::plugin
{
    QT_BEGIN_NAMESPACE
    namespace Ui
    {
        class SettingsWidget;
    }
    QT_END_NAMESPACE

    class SettingsWidget : public QWidget
    {
        Q_OBJECT

    public:
        explicit SettingsWidget(IBase *iBase, CENTAUR_INTERFACE_NAMESPACE::IConfiguration *config, QWidget *parent = nullptr);
        ~SettingsWidget() override;

    protected:
        void reloadTimer(const QString &timeframe);
        void reloadTimer(int index);
        void reloadTimer(double time);

    private:
        std::unique_ptr<Ui::SettingsWidget> ui;
        ExchangeRatePlugin *m_exchRatePlg { nullptr };
        CENTAUR_INTERFACE_NAMESPACE::IConfiguration *m_config { nullptr };
        QStandardItemModel *m_symsItemModel { nullptr };
        QStandardItemModel *m_usedItemModel { nullptr };
        QStandardItem *m_defCheckedItem { nullptr };
    };
} // namespace cen::plugin

#endif // CENTAUR_SETTINGSWIDGET_HPP
