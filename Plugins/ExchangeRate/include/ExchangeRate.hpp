/////////////////////////////////////////////////////////////////////////////////////
//
// Created by Ricardo Romero on 09/12/21.
// Copyright (c) 2021 Ricardo Romero.  All rights reserved.
//

#pragma once

#ifndef __cplusplus
#error "C++ compiler needed"
#endif /*__cplusplus*/

#ifndef CENTAUR_EXCHANGERATE_HPP
#define CENTAUR_EXCHANGERATE_HPP

#include <CentaurInterface.hpp>
#include <CentaurPlugin.hpp>
#include <QThread>

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"
#pragma clang diagnostic ignored "-Wextra-semi-stmt"
#pragma clang diagnostic ignored "-Wshadow"
#pragma clang diagnostic ignored "-Wambiguous-reversed-operator"
#pragma clang diagnostic ignored "-Wsuggest-override"
#pragma clang diagnostic ignored "-Wsuggest-destructor-override"
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif /*__clang__*/
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/schema.h>
#ifdef __clang__
#pragma clang diagnostic pop
#endif /*__clang__*/

#include <fstream>

namespace CENTAUR_PLUGIN_NAMESPACE
{
    class ValueThread : public QThread
    {
        Q_OBJECT
    public:
        ValueThread(QString qte, QString bse, qreal qteQty, QObject *parent) noexcept;
        ~ValueThread() override = default;

    public:
        void run() override;

    signals:
        void newValue(qreal val);

    private:
        QString quote;
        QString base;
        qreal quoteQuantity;
    };

    class ExchangeRatePlugin : public QObject,
                               public CENTAUR_PLUGIN_NAMESPACE::IExchangeRate
    {

        Q_OBJECT

        Q_PLUGIN_METADATA(IID "com.centaur-project.plugin.ExchangeRatePlugin/1.0")
        Q_INTERFACES(CENTAUR_PLUGIN_NAMESPACE::IBase CENTAUR_PLUGIN_NAMESPACE::IExchangeRate CENTAUR_PLUGIN_NAMESPACE::IStatus)

    public:
        explicit ExchangeRatePlugin(QObject *parent = nullptr);
        ~ExchangeRatePlugin() override = default;

        // IStatus
    public:
        DisplayMode initialize() noexcept override;
        QString text() noexcept override;
        QPixmap image() noexcept override;
        QFont font() noexcept override;
        QBrush brush(DisplayRole role) noexcept override;
        QAction *action() noexcept override;

        // IExchangeRate
    public:
        QList<QString> listSupported() noexcept override;
        qreal value(const QString &quote, const QString &base) noexcept override;
        qreal convert(const QString &quote, const QString &base, qreal quoteQuantity) noexcept override;
        qreal convert(const QString &quote, const QString &base, qreal quoteQuantity, QDate *date) noexcept override;

        // IBase
    public:
        QObject *getPluginObject() noexcept override;
        QString getPluginName() const noexcept override;
        QString getPluginVersionString() const noexcept override;
        void setPluginInterfaces(CENTAUR_INTERFACE_NAMESPACE::ILogger *logger, CENTAUR_INTERFACE_NAMESPACE::IConfiguration *config) noexcept override;
        uuid getPluginUUID() const noexcept override;
        QWidget *settingsWidget(IBase *thisObject, CENTAUR_INTERFACE_NAMESPACE::IConfiguration *config) const noexcept override;

    public:
        void loadData() noexcept;
        void acquireFromCache() noexcept;
        void acquireFromInternet() noexcept;
        void storeData() noexcept;
        void restartReloadTimer(int ms) noexcept;

    public:
        void onReloadData(bool threading) noexcept;

    protected slots:
        void onValueUpdate(qreal val) noexcept;

    public:
        rapidjson::Document pluginSettings;

    signals:
        void displayChange(plugin::IStatus::DisplayRole dr);

    private:
        QAction *m_clickAction;

        QTimer *m_reloadDataTimer;

        QMap<QString, QString> m_currencyData;
        QList<QPair<QString, QString>> m_currencySupported;

        uuid m_thisUUID;

        CENTAUR_INTERFACE_NAMESPACE::ILogger *m_logger { nullptr };
        CENTAUR_INTERFACE_NAMESPACE::IConfiguration *m_config { nullptr };

        QString m_defaultQuote;
        QString m_defaultBase;

        qreal m_defaultValue { -0.0 };

        int64_t m_updateMilliseconds { 0 };
        int64_t m_lastUpdateTimeStamp { 0 };

        bool m_configurationLoaded { false };
    };
} // namespace CENTAUR_PLUGIN_NAMESPACE

// Helper macros
#define logInfo(x, y) \
    m_logger->info(x, y)

#define logWarn(x, y) \
    m_logger->warning(x, y)

#if !defined(NDEBUG)
#define logTrace(x, y) \
    m_logger->trace(x, y)
#define logDebug(x, y) \
    m_logger->debug(x, y)
#else
#define logTrace(x, y) \
    (reinterpret_cast<void *>(0))
#define logDebug(x, y) \
    (reinterpret_cast<void *>(0))
#endif /**/

#define logError(x, y) \
    m_logger->error(x, y)

#define logFatal(x, y) \
    m_logger->fatal(x, y)

#endif // CENTAUR_EXCHANGERATE_HPP
