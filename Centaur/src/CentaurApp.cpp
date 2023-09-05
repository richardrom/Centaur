/////////////////////////////////////////////////////////////////////////////////////
//
// Created by Ricardo Romero on 06/12/21.
// Copyright (c) 2021 Ricardo Romero.  All rights reserved.
//

#include "CentaurApp.hpp"
#include "../ui/ui_CentaurApp.h"
#include "CandleChartDialog.hpp"
#include "CandleViewWidget.hpp"
#include "CentaurPlugin.hpp"
#include "DAL.hpp"
#include "DepthChartDialog.hpp"
#include "FavoritesDialog.hpp"
#include "Globals.hpp"
#include "LogDialog.hpp"
#include "Logger.hpp"
#include "LoginDialog.hpp"
#include "OrderbookDialog.hpp"
#include "SettingsDialog.hpp"
#include "SplashDialog.hpp"
#include "core-naming.hpp"
#include <QAreaSeries>
#include <QChartView>
#include <QDateTimeAxis>
#include <QDirIterator>
#include <QFileInfo>
#include <QGraphicsBlurEffect>
#include <QProcess>
#include <QPropertyAnimation>
#include <QShortcut>
#include <QSqlError>
#include <QSqlField>
#include <QTimer>
#include <QtCharts/QValueAxis>
#include <cui.hpp>
#include <utility>

/// \brief This structure hold the 'data' set to Impl::orderbookDepth and Impl::depthChart to properly
/// handle the visualization of data
struct OrderBookDepthInformation
{
    OrderBookDepthInformation()                                             = default;
    ~OrderBookDepthInformation()                                            = default;
    OrderBookDepthInformation(const OrderBookDepthInformation &)            = default;
    OrderBookDepthInformation &operator=(const OrderBookDepthInformation &) = default;

    OrderBookDepthInformation(QString sym, QString src) :
        symbol { std::move(sym) },
        source { std::move(src) } { }

    QString symbol;
    QString source;
};

Q_DECLARE_METATYPE(OrderBookDepthInformation);

BEGIN_CENTAUR_NAMESPACE

CentaurApp *g_app = nullptr;

struct CandleViewTimeFrameActions
{
    explicit CandleViewTimeFrameActions(QObject *parent);

public:
    std::map<CENTAUR_PLUGIN_NAMESPACE::TimeFrame, QAction *> actions;
};

struct CentaurApp::Impl
{
    explicit Impl(QMainWindow *parent) :
        ui { new Ui::CentaurApp },
        candleActions { new CandleViewTimeFrameActions(parent) },
        orderbookDepth { new QAction(QCoreApplication::tr("Orderbook"), parent) },
        depthChart { new QAction(QCoreApplication::tr("Depth chart"), parent) },
        removeWatchlist { new QAction(QCoreApplication::tr("Remove from watchlist"), parent) }
    {
        ui->setupUi(parent);
        logDialog = new LogDialog(parent);
    }

    std::unique_ptr<Ui::CentaurApp> ui;
    std::unique_ptr<CandleViewTimeFrameActions> candleActions;
    std::unique_ptr<std::thread> loggerThread { nullptr };
    std::unique_ptr<ProtocolServer> server;

    LogDialog *logDialog { nullptr };
    QChart *last7Chart { nullptr };
    QChartView *sevenDayGraph { nullptr };

    QAction *orderbookDepth;
    QAction *depthChart;
    QAction *removeWatchlist;

    std::vector<CENTAUR_PLUGIN_NAMESPACE::IBase *> pluginsData;

    std::pair<QString, QString> currentWatchListSelection;

    PluginConfigurationMap configurationInterface; /// Every PluginConfiguration mapped to the Plugin UUID
    PluginExchangesMap exchangeList;               /// Every ExchangeInformation (IExchanges plugin and their basic information) map
    QList<QPluginLoader *> pluginInstances;        /// All instances of the plugins
    std::map<uuid, QList<QAction *>> exchangeMenuActions;
    std::map<uuid, std::tuple<CENTAUR_PLUGIN_NAMESPACE::IStatus *, QToolButton *, CENTAUR_PLUGIN_NAMESPACE::IStatus::DisplayMode>> statusPlugins;

    QAreaSeries *last7SevenAreaSeries { nullptr };
    QSplineSeries *last7SevenLowSeries { nullptr };
    QSplineSeries *last7SevenUpSeries { nullptr };

    QTimer *credentialsTimer { nullptr };
};

CandleViewTimeFrameActions::CandleViewTimeFrameActions(QObject *parent) :
    actions {
        { CENTAUR_PLUGIN_NAMESPACE::TimeFrame::Seconds_1,   new QAction(QCoreApplication::tr("1 second"), parent)},
        { CENTAUR_PLUGIN_NAMESPACE::TimeFrame::Seconds_5, new QAction(QCoreApplication::tr(" 5 seconds"), parent)},
        {CENTAUR_PLUGIN_NAMESPACE::TimeFrame::Seconds_10, new QAction(QCoreApplication::tr("10 seconds"), parent)},
        {CENTAUR_PLUGIN_NAMESPACE::TimeFrame::Seconds_30, new QAction(QCoreApplication::tr("30 seconds"), parent)},
        {CENTAUR_PLUGIN_NAMESPACE::TimeFrame::Seconds_45, new QAction(QCoreApplication::tr("45 seconds"), parent)},
        { CENTAUR_PLUGIN_NAMESPACE::TimeFrame::Minutes_1,   new QAction(QCoreApplication::tr("1 minute"), parent)},
        { CENTAUR_PLUGIN_NAMESPACE::TimeFrame::Minutes_2,  new QAction(QCoreApplication::tr("2 minutes"), parent)},
        { CENTAUR_PLUGIN_NAMESPACE::TimeFrame::Minutes_3,  new QAction(QCoreApplication::tr("3 minutes"), parent)},
        { CENTAUR_PLUGIN_NAMESPACE::TimeFrame::Minutes_5,  new QAction(QCoreApplication::tr("5 minutes"), parent)},
        {CENTAUR_PLUGIN_NAMESPACE::TimeFrame::Minutes_10, new QAction(QCoreApplication::tr("10 minutes"), parent)},
        {CENTAUR_PLUGIN_NAMESPACE::TimeFrame::Minutes_15, new QAction(QCoreApplication::tr("15 minutes"), parent)},
        {CENTAUR_PLUGIN_NAMESPACE::TimeFrame::Minutes_30, new QAction(QCoreApplication::tr("30 minutes"), parent)},
        {CENTAUR_PLUGIN_NAMESPACE::TimeFrame::Minutes_45, new QAction(QCoreApplication::tr("45 minutes"), parent)},
        {   CENTAUR_PLUGIN_NAMESPACE::TimeFrame::Hours_1,     new QAction(QCoreApplication::tr("1 hour"), parent)},
        {   CENTAUR_PLUGIN_NAMESPACE::TimeFrame::Hours_2,    new QAction(QCoreApplication::tr("2 hours"), parent)},
        {   CENTAUR_PLUGIN_NAMESPACE::TimeFrame::Hours_4,    new QAction(QCoreApplication::tr("4 hours"), parent)},
        {   CENTAUR_PLUGIN_NAMESPACE::TimeFrame::Hours_6,    new QAction(QCoreApplication::tr("6 hours"), parent)},
        {   CENTAUR_PLUGIN_NAMESPACE::TimeFrame::Hours_8,    new QAction(QCoreApplication::tr("8 hours"), parent)},
        {  CENTAUR_PLUGIN_NAMESPACE::TimeFrame::Hours_12,   new QAction(QCoreApplication::tr("12 hours"), parent)},
        {    CENTAUR_PLUGIN_NAMESPACE::TimeFrame::Days_1,      new QAction(QCoreApplication::tr("1 day"), parent)},
        {    CENTAUR_PLUGIN_NAMESPACE::TimeFrame::Days_3,     new QAction(QCoreApplication::tr("3 days"), parent)},
        {   CENTAUR_PLUGIN_NAMESPACE::TimeFrame::Weeks_1,     new QAction(QCoreApplication::tr("1 week"), parent)},
        {  CENTAUR_PLUGIN_NAMESPACE::TimeFrame::Months_1,    new QAction(QCoreApplication::tr("1 month"), parent)}
}
{
}

CentaurApp::CentaurApp(QWidget *parent) :
    QMainWindow(parent),
    _impl { new Impl(this) }
{
    setWindowFlag(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    std::unique_ptr<SplashDialog> splashScreen = std::make_unique<SplashDialog>(this);
    splashScreen->setDisplayText(tr("Setting up user interface"));
    splashScreen->setProgressRange(0, 7);
    splashScreen->setProgressPos(0);

    START_TIME(initializationTimeStart);

    g_app     = this;
    g_globals = new Globals;

    initSession();

    splashScreen->show();
    qRegisterMetaType<uuid>("uuid");
    // qRegisterMetaType<cen::plugin::CandleData>("plugin::ICandleView::CandleData");
    qRegisterMetaType<cen::plugin::TimeFrame>("plugin::ICandleView::TimeFrame");
    qRegisterMetaType<cen::plugin::IExchange::Timestamp>("plugin::ICandleView::Timestamp");
    qRegisterMetaType<plugin::IStatus::DisplayRole>("plugin::IStatus::DisplayRole");

#ifdef Q_OS_MAC
    CFURLRef appUrlRef       = CFBundleCopyBundleURL(CFBundleGetMainBundle());
    CFStringRef macPath      = CFURLCopyFileSystemPath(appUrlRef, kCFURLPOSIXPathStyle);
    g_globals->paths.appPath = CFStringGetCStringPtr(macPath, CFStringGetSystemEncoding());
    CFRelease(appUrlRef);
    CFRelease(macPath);

    // TODO: On Final Release. Plugins Path must be in the Application Data path
    g_globals->paths.pluginsPath = g_globals->paths.appPath + "/Contents/Plugins";

#else

#endif // Q_OS_MAC

    g_globals->paths.resPath = g_globals->paths.appPath + "/Contents/Resources";

    // Start logging service
    startLoggingService();
    splashScreen->step();

    // Init the database internal file
    initializeDatabaseServices();
    splashScreen->step();

    // Restore file
    loadInterfaceState();
    splashScreen->step();

    // Start the interface
    initializeInterface();
    initializeShortcuts();
    splashScreen->step();

    // Start the server
    splashScreen->setDisplayText(tr("Starting the communication server"));
    // startCommunicationsServer();
    splashScreen->step();

    // Load plugins
    loadPlugins(splashScreen.get());
    splashScreen->step();

    // Load the "favorites list". Since all plugins must be loaded
    splashScreen->setDisplayText(tr("Adding favorites watchlist"));
    loadFavoritesWatchList();
    splashScreen->step();

    END_TIME_SEC(initializationTimeStart, initializationTimeEnd, initializationTime);
    logInfo("app", QString("Opening time: ##00BFFF#%1#").arg(initializationTime.count(), 0, 'f', 4));
    splashScreen->hide();
}

CentaurApp::~CentaurApp()
{
    for (auto &pli : _impl->configurationInterface) {
        delete pli.second;
    }
    if (g_logger != nullptr) {
        g_logger->terminate();
        _impl->loggerThread->join();
    }

    delete g_credentials;
    delete g_globals;
}

void CentaurApp::initSession()
{
    if (SettingsDialog::isFirstTimeStarted()) {
        const auto res = QMessageBox::question(this, tr("User"), tr("There is no user information. Would you like to set it?"), QMessageBox::Yes | QMessageBox::No);
        if (res == QMessageBox::No) {
            exit(EXIT_FAILURE);
        }
        else {
            onShowSettings();

            QMessageBox::warning(this,
                "Restart",
                "Application will be restarted",
                QMessageBox::Ok);

            // Restart the App
            QProcess::startDetached(QCoreApplication::arguments()[0], QCoreApplication::arguments());
            exit(EXIT_SUCCESS);
        }
    }
    else {

        LoginDialog dlg;
        dlg.setNormalMode();
#ifndef TEST_LOGIN_MODE
        if (dlg.exec() != QDialog::Accepted) {
            exit(EXIT_FAILURE);
        }

#else
        dlg.onAccept();
#endif
        updateUserInformationStatus();
    }
}

void CentaurApp::credentialsStatus() noexcept
{
    delete g_credentials;
    g_credentials = nullptr;

    logInfo("credentialsStatus", "Credentials no longer in cache");
}

void CentaurApp::keepAliveCredentialStatus() noexcept
{
    // TODO Set an advance setting for this
    QSettings settings;
    settings.beginGroup("__Session__data");
    const auto time = settings.value("__securing_time__", 300'000).toInt();
    settings.endGroup();

    if (_impl->credentialsTimer == nullptr) {
        _impl->credentialsTimer = new QTimer(this);
        _impl->credentialsTimer->setSingleShot(true);
        connect(_impl->credentialsTimer, &QTimer::timeout, this, &CentaurApp::credentialsStatus);
    }

    _impl->credentialsTimer->start(time);
}

Ui::CentaurApp *CentaurApp::ui()
{
    return _impl->ui.get();
}

LogDialog *CentaurApp::logDialog() noexcept
{
    return _impl->logDialog;
}

void CentaurApp::mapPluginBase(CENTAUR_PLUGIN_NAMESPACE::IBase *base) noexcept
{
    _impl->pluginsData.emplace_back(base);
}

void CentaurApp::removeLastPluginBase() noexcept
{
    _impl->pluginsData.pop_back();
}

void CentaurApp::mapConfigurationInterface(const uuid &id, PluginConfiguration *config)
{
    _impl->configurationInterface[id] = config;
}

void CentaurApp::mapExchangePlugin(const uuid &id, const ExchangeInformation &info)
{
    _impl->exchangeList[id] = info;
}

void CentaurApp::mapPluginInstance(QPluginLoader *loader)
{
    _impl->pluginInstances.push_back(loader);
}

void CentaurApp::mapExchangePluginViewMenus(const uuid &plugin, const QList<QAction *> &actions)
{
    _impl->exchangeMenuActions[plugin] = actions;
}

void CentaurApp::mapStatusPlugins(const uuid &plugin, CENTAUR_PLUGIN_NAMESPACE::IStatus *status, QToolButton *button, CENTAUR_PLUGIN_NAMESPACE::IStatus::DisplayMode mode)
{
    _impl->statusPlugins[plugin] = { status, button, mode };
}

std::vector<CENTAUR_PLUGIN_NAMESPACE::IBase *> &CentaurApp::getPluginBase() const noexcept
{
    return _impl->pluginsData;
}

PluginConfiguration *CentaurApp::getPluginConfig(const uuid &id) const noexcept
{
    auto iter = _impl->configurationInterface.find(id);
    if (iter == _impl->configurationInterface.end())
        return nullptr;
    return iter->second;
}

void CentaurApp::closeEvent(QCloseEvent *event)
{
    if (_impl->server) {
        // Stop the server
        _impl->server->close();
        _impl->server.reset();
    }

    for (const auto &plugins : _impl->pluginInstances) {
        if (plugins->isLoaded())
            plugins->unload();
    }

    saveInterfaceState();

    event->accept();
}

void CentaurApp::initializeDatabaseServices() noexcept
{
    using namespace dal;
    auto status = DataAccess::openDatabase(this);

    switch (status) {
        case OpenDatabaseCode::Fatal:
            {
                QMessageBox::critical(this, tr("Error"), tr("The Centaur database could not be opened."));
                exit(EXIT_FAILURE);
            }

        case OpenDatabaseCode::Recreate:
            {
                status = DataAccess::createDatabase(this);
                if (status == OpenDatabaseCode::Fatal)
                    exit(EXIT_FAILURE);
            }
            break;
        case OpenDatabaseCode::Ok:
            [[likely]];
    }
}

void CentaurApp::initializeInterface() noexcept
{
    logTrace("app", "CentaurApp::initializeInterface()");

    ui()->mainWindowFrame->overrideMovableParent(this);

#ifdef DEBUG
    const QString applicationTitle = QString("%1 %2").arg(cen::defines::_application_Name, CentaurVersionString);
#else
    const QString applicationTitle = QString("%1").arg(cen::defines::_application_Name);
#endif
    ui()->mainFrameTitle->setFrameTitle(applicationTitle);

    auto *viewPluginsButton      = ui()->mainFrameTitle->createToolButton("viewPluginsButton");
    auto *viewLogsButton         = ui()->mainFrameTitle->createToolButton("viewLogsButton");
    auto *viewServerStatusButton = ui()->mainFrameTitle->createToolButton("viewServerStatusButton");

    viewPluginsButton->setToolTip(tr("Show the plugins settings"));
    viewLogsButton->setToolTip(tr("Show the Logs Window"));
    viewServerStatusButton->setToolTip(tr("Show the internal server status"));

    auto loadIcon = [](const QString &url) {
        QIcon pluginIcon;
        pluginIcon.addFile(url, QSize(), QIcon::Normal, QIcon::Off);
        return pluginIcon;
    };

    // The normal state of the side panel is in the visible state
    // So, only toggle the view of the panel if its was store as contracted
    if (m_sidePanelContracted) {
        const bool currentAnimationStatus = m_useAnimations;
        // Prevent use animations in the early initialization step
        m_useAnimations = false;
        // Toggle the flag, so onToggleSidePanel() hides the bar
        m_sidePanelContracted = !m_sidePanelContracted;
        onToggleSidePanel();
        m_useAnimations = currentAnimationStatus;
    }

    viewPluginsButton->setIcon(loadIcon(QString::fromUtf8(":/svg/commands/extension")));
    viewLogsButton->setIcon(loadIcon(QString::fromUtf8(":/svg/commands/terminal")));
    viewServerStatusButton->setIcon(loadIcon(QString::fromUtf8(":/svg/commands/server-red")));

    connect(ui()->settingsButton, &QPushButton::released, this, &CentaurApp::onShowSettings);
    for (auto &action : _impl->candleActions->actions) {
        auto &[tmFrame, act] = action;
        connect(act, &QAction::triggered, this, [&, _tmFrame = tmFrame]() {
            const auto actionData = _impl->orderbookDepth->data().value<OrderBookDepthInformation>();
            onViewCandleChart(actionData.symbol, actionData.source, _tmFrame);
        });
    }

    // Connect favorites action
    connect(ui()->actionWatchlistFavorites,
        &QAction::triggered,
        this,
        [&](C_UNUSED bool triggered) {
            FavoritesDialog dlg(this);
            dlg.setModal(true);
            dlg.exec();
        });

    connect(ui()->watchlistButton, &QPushButton::clicked, this, [&](bool clicked) {
        if (clicked)
            ui()->stackedWidget->setCurrentWidget(ui()->watchListPage);
    });

    connect(_impl->orderbookDepth, &QAction::triggered, this, [&, action = _impl->orderbookDepth](C_UNUSED bool trigger) {
        const auto actionData    = _impl->orderbookDepth->data().value<OrderBookDepthInformation>();
        const QString objectName = QString("%1%2orderbook").arg(actionData.source, actionData.symbol);

        auto *dlgExists = this->findChild<OrderbookDialog *>(objectName);
        if (dlgExists != nullptr) {
            // Bring upfront
            dlgExists->show();
            dlgExists->setWindowState(Qt::WindowState::WindowActive);
        }
        else {
            auto *dlg = new OrderbookDialog(actionData.symbol, _impl->exchangeList[uuid(actionData.source.toStdString(), false)].exchange, this);
            dlg->setObjectName(objectName);
            connect(dlg, &OrderbookDialog::closeButtonPressed, this, [&, dlg]() {
                delete dlg;
            });

            dlg->show();
        }
    });

    connect(_impl->depthChart, &QAction::triggered, this, [&, action = _impl->depthChart](C_UNUSED bool trigger) {
        const auto actionData             = _impl->depthChart->data().value<OrderBookDepthInformation>();
        const QString orderBookObjectName = QString("%1%2orderbook").arg(actionData.source, actionData.symbol);

        auto *orderBookExists = this->findChild<OrderbookDialog *>(orderBookObjectName);
        if (orderBookExists == nullptr) {
            QMessageBox::warning(this, tr("Warning"), tr("The orderbook has to be displayed"), QMessageBox::Ok);
            return;
        }

        const QString objectName = QString("%1%2depth").arg(actionData.source, actionData.symbol);
        auto *dlgExists          = this->findChild<DepthChartDialog *>(objectName);
        if (dlgExists != nullptr) {
            // Bring upfront
            dlgExists->show();
            dlgExists->setWindowState(Qt::WindowState::WindowActive);
        }
        else {
            auto *dlg = new DepthChartDialog(actionData.symbol, this);
            dlg->setObjectName(objectName);

            connect(orderBookExists, &OrderbookDialog::redirectOrderbook, dlg, &DepthChartDialog::onOrderBookDepth);

            connect(dlg, &DepthChartDialog::closeButtonPressed, this, [&, dlg, orderBookExists]() {
                // disconnect the orderbook signal
                disconnect(orderBookExists, &OrderbookDialog::redirectOrderbook, nullptr, nullptr);
                delete dlg;
            });

            dlg->show();
        }
    });

    connect(_impl->removeWatchlist, &QAction::triggered, this, [&](C_UNUSED bool trigger) {
        const auto actionData = _impl->depthChart->data().value<OrderBookDepthInformation>();
        onRemoveWatchList(actionData.source, actionData.symbol);
    });

    ui()->porfolioList->linkSearchEdit(ui()->porfolioSearch);

    //    QPixmap pm;
    //    g_globals->symIcons.find(32, "BTC", &pm, Globals::AssetIcons::Crypto, this);
    //    ui()->porfolioList->insertItem(pm, "Bitcoin", "Binance SPOT", "BTC", "$ 45.02 USD", "0.023 BTC");
    //    g_globals->symIcons.find(32, "ETH", &pm, Globals::AssetIcons::Crypto, this);
    //    ui()->porfolioList->insertItem(pm, "Ethereum", "Binance SPOT", "ETH", "$ 450.31 USD", "0.25 ETH");
    //    g_globals->symIcons.find(32, "BNB", &pm, Globals::AssetIcons::Crypto, this);
    //    ui()->porfolioList->insertItem(pm, "Binance Coin", "Binance BNB", "BNB", "$ 125.02 USD", "0.505 BNB");
    //    g_globals->symIcons.find(32, "TRX", &pm, Globals::AssetIcons::Crypto, this);
    //    ui()->porfolioList->insertItem(pm, "Tron", "Binance SPOT", "TRX", "$ 85.97 USD", "1250.2548 TRX");
    //    g_globals->symIcons.find(32, "DOT", &pm, Globals::AssetIcons::Crypto, this);
    //    ui()->porfolioList->insertItem(pm, "Polkadot", "Binance SPOT", "DOT", "$ 0.23 USD", "0.0023 DOT");

    ui()->watchListWidget->linkSearchEdit(ui()->watchListSearch);

    connect(ui()->watchListWidget, &WatchlistWidget::mouseRightClick, this, [&](const QPoint &pt) {
        const auto &[symbol, source] = ui()->watchListWidget->sourceFromPoint(pt);

        if (symbol.isEmpty() || source.isEmpty())
            return;

        const QPoint menuPoint = ui()->watchListWidget->mapToGlobal(pt);

        auto actions = _impl->exchangeMenuActions[uuid { source.toStdString(), false }];

        QMenu menu(this);

        auto *exchBase = _impl->exchangeList[uuid { source.toStdString(), false }].exchange;

        const OrderBookDepthInformation obdi { symbol, source };

        _impl->orderbookDepth->setData(QVariant::fromValue(obdi));
        _impl->depthChart->setData(QVariant::fromValue(obdi));
        _impl->removeWatchlist->setData(QVariant::fromValue(obdi));

        menu.addAction(_impl->orderbookDepth);
        menu.addAction(_impl->depthChart);
        menu.addSeparator();

        if (exchBase != nullptr) {
            const auto pluginInformation = pluginInformationFromBase(exchBase);
            auto candleView              = exchBase->supportedTimeFrames();
            if (!candleView.empty()) {
                QMenu *candleMenu = menu.addMenu("Charts");

                for (const auto &cd : candleView) {
                    if (cd == plugin::TimeFrame::nullTime)
                        candleMenu->addSeparator();
                    else {
                        auto iter = _impl->candleActions->actions.find(cd);
                        if (iter != _impl->candleActions->actions.end()) {
                            iter->second->setData(QVariant::fromValue(obdi));
                            candleMenu->addAction(iter->second);
                        }
                    }
                }
                menu.addSeparator();
            }
        }

        for (auto &action : actions) {
            // Update the symbol name

            if (action->isSeparator()) {
                menu.addSeparator();
            }
            else {
                action->setData(symbol);
                menu.addAction(action);
            }
        }
        if (!actions.isEmpty())
            menu.addSeparator();

        menu.addAction(_impl->removeWatchlist);

        menu.exec(menuPoint);
    });

    _impl->sevenDayGraph = new QChartView(this);
    _impl->sevenDayGraph->setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::Dialog);
    _impl->sevenDayGraph->setStyleSheet(R"(QFrame{
background-color: qlineargradient(x1:0.5, y1: 0, x2:0.5, y2:1, stop: 1  rgba(53,61,66,50), stop: 0 rgba(79,85,90,50));
border-top-left-radius: 10px;
border-top-right-radius: 10px;
border-bottom-left-radius: 10px;
border-bottom-right-radius: 10px;
border: 0px;
})");
#ifdef Q_OS_MAC
    _impl->sevenDayGraph->setAttribute(Qt::WA_TranslucentBackground);
#endif

    connect(ui()->watchListWidget, &WatchlistWidget::itemHover, this, [&](const QString &symbol, const QString &source) {
        if (symbol.isEmpty() || source.isEmpty()) {
            _impl->sevenDayGraph->hide();
            return;
        }

        auto itemIter = _impl->exchangeList.find(uuid(source.toStdString(), false));
        if (itemIter == _impl->exchangeList.end()) {
            logError("wlOrderbookSend", QString("Watchlist item for the symbol %1 was not found").arg(symbol));
            return;
        }

        const auto &exchInfo = itemIter->second;
        auto origin          = ui()->watchListWidget->mapToGlobal(ui()->watchListWidget->frameGeometry().topRight()); // To get the right

        plotSevenDaysChart(symbol, exchInfo.exchange->get7dayData(symbol));

        _impl->sevenDayGraph->setGeometry(
            origin.x(),
            origin.y(),
            400, 400);

        _impl->sevenDayGraph->show();
    });

    connect(ui()->watchListWidget, &WatchlistWidget::mouseLeave, this, [&]() {
        _impl->sevenDayGraph->hide();
    });

    // Init Last-7-day chart
    _impl->last7Chart = new QChart;
    _impl->last7Chart->setAnimationOptions(QChart::AllAnimations);
    _impl->last7Chart->setBackgroundPen(Qt::NoPen);
    _impl->last7Chart->setBackgroundBrush(QColor(3, 12, 19));
    _impl->last7Chart->setTitleBrush(QColor(200, 200, 200));
    _impl->sevenDayGraph->setChart(_impl->last7Chart);
    _impl->sevenDayGraph->setRenderHint(QPainter::Antialiasing);
    _impl->last7SevenLowSeries  = new QSplineSeries;
    _impl->last7SevenUpSeries   = new QSplineSeries;
    _impl->last7SevenAreaSeries = new QAreaSeries(_impl->last7SevenLowSeries, _impl->last7SevenUpSeries);
    _impl->last7SevenAreaSeries->setPen(Qt::NoPen);
    _impl->last7Chart->legend()->hide();

    _impl->last7Chart->addSeries(_impl->last7SevenAreaSeries);

    auto *axisX = new QDateTimeAxis;
    axisX->setTickCount(7);
    axisX->setFormat("MMM d");
    axisX->setTitleVisible(false);
    axisX->setGridLineVisible(false);
    axisX->setLabelsColor(QColor(200, 200, 200));
    _impl->last7Chart->addAxis(axisX, Qt::AlignBottom);
    _impl->last7SevenAreaSeries->attachAxis(axisX);

    auto *axisY = new QValueAxis;
    axisY->setLabelFormat("$ %.2f");
    axisY->setTitleVisible(false);
    axisY->setGridLineVisible(false);
    axisY->setLabelsColor(QColor(200, 200, 200));
    _impl->last7Chart->addAxis(axisY, Qt::AlignLeft);
    _impl->last7SevenAreaSeries->attachAxis(axisY);

    _impl->last7Chart->setMargins(QMargins(0, 0, 0, 0));
}

void CentaurApp::initializeShortcuts() noexcept
{
    namespace json = rapidjson;

    QSettings settings;
    settings.beginGroup("Settings.Keymap");
    const auto savedKeymap = settings.value("keymap", ".").toString();
    settings.endGroup();

    // Load the schema file
    const QString schemaJSONKeymap = g_globals->paths.resPath + "/Schema/keymap.schema.json";
    QFile file(schemaJSONKeymap);
    if (!file.open(QIODevice::ReadOnly)) {
        const QString message = tr("An internal file is missing for the keyboard shortcuts. Reinstalling the application might solve the problem");

        logError("initializeShortcuts", message);

        QMessageBox::critical(this,
            tr("Missing file"),
            message,
            QMessageBox::Ok);

        return;
    }

    QTextStream textStream(&file);
    json::Document schemaJSONDoc;
    schemaJSONDoc.Parse(textStream.readAll().toUtf8().constData());
    if (schemaJSONDoc.HasParseError()) {
        const QString message = tr("An internal file is corrupted for the keyboard shortcuts. Reinstalling the application might solve the problem");

        logError("initializeShortcuts", message);

        QMessageBox::critical(this,
            tr("Corrupted file"),
            message,
            QMessageBox::Ok);

        return;
    }

    const json::SchemaDocument schemaDoc(schemaJSONDoc);
    json::SchemaValidator schemaValidator(schemaDoc);

    // Read the json files
    const QStringList searchPaths {
        g_globals->paths.resPath + "/Keymap/",                                              // internal files
        QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/keymap/" // user defined
    };

    for (const auto &path : searchPaths) {
        QDirIterator it(path, QDirIterator::NoIteratorFlags);
        while (it.hasNext()) {
            const QFileInfo nfo(it.next());
            if (nfo.completeSuffix() != "keymap.json") {
                logWarn("initializeShortcuts", tr("An extraneous file (%1) in a keymap path (%2) was found").arg(nfo.fileName(), path));
                continue;
            }

            QFile jsonKeymap(nfo.absoluteFilePath());
            if (!jsonKeymap.open(QIODevice::ReadOnly)) {
                logWarn("initializeShortcuts", tr("File %1 in the keymap path (%2) could not be opened").arg(nfo.fileName(), path));
                continue;
            }

            QTextStream jsonFileData(&jsonKeymap);

            json::Document jsonDoc;
            jsonDoc.Parse(jsonFileData.readAll().toUtf8().constData());

            if (jsonDoc.HasParseError()) {
                logWarn("initializeShortcuts", tr("File %1 in the keymap path (%2) is not a valid JSON file").arg(nfo.fileName(), path));
                continue;
            }

            if (!jsonDoc.Accept(schemaValidator)) {
                logWarn("initializeShortcuts", tr("File %1 in the keymap path (%2) is not a valid keymap json file").arg(nfo.fileName(), path));
                continue;
            }

            const QString keymapName { jsonDoc["name"].GetString() };

            if (savedKeymap == keymapName) {
                loadShortcuts(jsonDoc);
                return;
            }
        }
    }
}

void CentaurApp::loadShortcuts(const rapidjson::Document &document) noexcept
{

    auto doConnection = [&](auto &json, auto &&func) {
        auto *shortcut = new QShortcut(QKeySequence::fromString(json["shortcut"].GetString(), QKeySequence::PortableText), this);

        this->connect(shortcut, &QShortcut::activated, this, func);
    };

    for (const auto &wnd : document["keymap"]["Window"].GetArray()) {
        if (wnd["id"] == "open-settings") {
            doConnection(wnd, &CentaurApp::onShowSettings);
        }
    }

    for (const auto &tls : document["keymap"]["Tools"].GetArray()) {
        if (tls["id"] == "open-plugins-info") {
            doConnection(tls, &CentaurApp::onShowPlugins);
        }
        else if (tls["id"] == "open-log-window") {
            doConnection(tls, &CentaurApp::onShowLogDialog);
        }
    }
}

void CentaurApp::saveInterfaceState() noexcept
{
    logTrace("app", "CentaurApp::saveInterfaceState()");

    QSettings settings;

    settings.beginGroup("mainWindow");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("state", saveState());
    settings.endGroup();

    settings.beginGroup("animations");
    settings.setValue("use", m_useAnimations);
    settings.endGroup();

    settings.beginGroup("sidePanel");
    settings.setValue("contracted", m_sidePanelContracted);
    settings.endGroup();

    _impl->logDialog->saveInterface();

    logInfo("app", "UI state saved");
}

void CentaurApp::loadInterfaceState() noexcept
{
    logTrace("app", "CentaurApp::loadInterfaceState()");

    QSettings settings;

    settings.beginGroup("mainWindow");
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("state").toByteArray());
    settings.endGroup();

    // Load advanced settings
    settings.beginGroup("advancedSettings-PixmapCache");
    // Always write the default
    settings.setValue("default", QPixmapCache::cacheLimit());
    const int pixCacheValue = settings.value("size", -1).toInt();
    if (pixCacheValue == -1 || pixCacheValue > 1'073'741'824 || pixCacheValue < 10'240'000) {
        logError("loadInterfaceState", QString(tr("Pixmap Cache Memory Value is not correct (%1)")).arg(pixCacheValue));
    }
    else {
        QPixmapCache::setCacheLimit(pixCacheValue);
    }
    settings.endGroup();

    settings.beginGroup("animations");
    m_useAnimations = settings.value("use", true).toBool();
    settings.endGroup();

    settings.beginGroup("sidePanel");
    m_sidePanelContracted = settings.value("contracted", false).toBool();
    settings.endGroup();

    logInfo("app", "UI state loaded");
}

void CentaurApp::onStatusDisplayChanged(plugin::IStatus::DisplayRole mode)
{
    using namespace CENTAUR_PLUGIN_NAMESPACE;
    plugin::IStatus *sndr = qobject_cast<plugin::IStatus *>(sender());
    if (sndr == nullptr) {
        logError("IStatus", "The IStatus sender could not be determinate");
        return;
    }

    auto data = _impl->statusPlugins.find(sndr->getPluginUUID());
    if (data == _impl->statusPlugins.end()) {
        logError("IStatus", "The IStatus data could not be found");
        return;
    }

    QToolButton *button = std::get<1>(data->second);
    if (mode == plugin::IStatus::DisplayRole::Foreground) {
        button->setFont(sndr->font());
        button->setText(sndr->text());

        constexpr static char stylesheet[] = { R"(QToolButton{border-radius: 0px; border: 0px; }
QToolButton:hover{background-color: qlineargradient(x1:0.5, y1: 0, x2:0.5, y2:1, stop: 0  rgb(58,58,58), stop: 1 rgb(68,68,68)); border-radius: 0px;}
QToolButton:pressed{background-color: qlineargradient(x1:0.5, y1: 0, x2:0.5, y2:1, stop: 1  rgb(85,85,85), stop: 0 rgb(95,95,95)); border-radius: 0px;})" };

        const auto foregroundBrush = sndr->brush(IStatus::DisplayRole::Foreground);
        const auto backgroundBrush = sndr->brush(IStatus::DisplayRole::Background);

        QPalette palette = button->palette();
        if (foregroundBrush != Qt::NoBrush) {
            palette.setBrush(QPalette::ColorRole::ButtonText, sndr->brush(IStatus::DisplayRole::Foreground));
        }
        if (backgroundBrush != Qt::NoBrush) {
            button->setStyleSheet("");
            button->setAutoFillBackground(true);
            palette.setBrush(QPalette::ColorRole::Button, sndr->brush(IStatus::DisplayRole::Background));
        }
        else {
            button->setStyleSheet(stylesheet);
            button->setAutoFillBackground(false);
        }
        button->setPalette(palette);
    }
    else if (mode == plugin::IStatus::DisplayRole::Text) {
        button->setText(sndr->text());
    }
    // TODO: all other modes
}

void CentaurApp::loadFavoritesWatchList() noexcept
{
    auto data = dal::DataAccess::selectFavoriteSymbols();

    if (data->empty()) {
        logInfo("app", tr("No favorites in the list"));
        return;
    }

    for (const auto &[sym, plid] : *data) {
        if (sym.isEmpty()) {
            logWarn("app", tr("The symbol name in the favorites DB is empty"));
            continue;
        }

        if (plid.isEmpty()) {
            logWarn("app", tr("The symbol id in the favorites DB is empty"));
            continue;
        }

        // Insert the element
        onAddToWatchList(sym, plid, false);
    }
}

void CentaurApp::startLoggingService() noexcept
{
    g_logger = new CentaurLogger;
    // Init the logger
    _impl->loggerThread = std::make_unique<std::thread>(&CentaurLogger::run, g_logger);
    try {
        const auto &user = g_globals->session.user;
        g_logger->setApplication(this);
        g_logger->setUser(user.isEmpty() ? "root" : user);
        logInfo("app", QString("TraderSys ") + QString(CentaurVersionString));
    } catch (const std::runtime_error &ex) {
        QMessageBox::critical(this,
            tr("Fatal error"),
            ex.what());
        QApplication::quit();
    }
}

void CentaurApp::startCommunicationsServer() noexcept
{
    auto *statusButton = findChild<QToolButton *>("viewServerStatusButton");
    _impl->server      = std::make_unique<ProtocolServer>(this);
    QIcon icon;
    if (_impl->server->isListening()) {
        icon.addFile(QString::fromUtf8(":/svg/commands/server-green"), QSize(), QIcon::Normal, QIcon::Off);
        if (statusButton)
            statusButton->setIcon(icon);
    }
    else {
        icon.addFile(QString::fromUtf8(":/svg/commands/server-red"), QSize(), QIcon::Normal, QIcon::Off);
        if (statusButton)
            statusButton->setIcon(icon);
    }
}

void CentaurApp::onAddToWatchList(const QString &symbol, const QString &sender, bool addToDatabase) noexcept
{
    logTrace("watchlist", "CentaurApp::onAddToWatchList()");

    auto interface = _impl->exchangeList.find(uuid { sender.toStdString(), false });

    if (interface == _impl->exchangeList.end()) {
        logError("watchlist", QString(tr("The sender %1 is not registered.")).arg(sender));
        return;
    }

    auto watchInterface = interface->second.exchange;

    // Generate a unique-id
    const auto [addSuccess, symbolWatchlistPixmap] = watchInterface->addSymbolToWatchlist(symbol);
    if (addSuccess) {
        // QPixmap icon;
        // qDebug() << icon << watchInterface->getBaseFromSymbol(symbol);
        ui()->watchListWidget->insertItem(symbolWatchlistPixmap, symbol, sender, 0.0, 0.0, 0);

        // Add to the database
        if (addToDatabase)
            dal::DataAccess::addSymbolToFavorites(symbol, sender);

        // TODO: move the next code to a separate function (MAY BE) need more analysis
        QLinearGradient gr_p({ 0, 0 }, { 1, 1 });
        gr_p.setCoordinateMode(QGradient::CoordinateMode::ObjectMode);
        gr_p.setColorAt(0, QColor(1, 166, 7));
        gr_p.setColorAt(0.5, QColor(26, 175, 32));
        gr_p.setColorAt(1, QColor(1, 133, 6));

        QLinearGradient gr_n({ 0, 0 }, { 1, 1 });
        gr_n.setCoordinateMode(QGradient::CoordinateMode::ObjectMode);
        gr_n.setColorAt(0, QColor(167, 29, 7));
        gr_n.setColorAt(0.5, QColor(176, 52, 32));
        gr_n.setColorAt(1, QColor(134, 23, 6));

        auto dataList = watchInterface->getWatchlist24hrPriceChange();

        std::transform(dataList.begin(), dataList.end(), dataList.begin(),
            [&sender, &ui = _impl->ui](const std::tuple<qreal, qreal, QString> &data) -> std::tuple<qreal, qreal, QString> {
                ui->watchListWidget->updateDifference(std::get<2>(data), sender, std::get<1>(data));
                return { std::get<0>(data),
                    std::get<1>(data),
                    QString(R"(<font size="12"><b>%1</b></font><br><font size="11"><b>$ %2</b><br>%3 %</font>)")
                        .arg(std::get<2>(data), QLocale(QLocale::English).toString(std::get<0>(data), 'f', 2))
                        .arg(std::get<1>(data), 'f', '2') };
            });

        ui()->widget->setData(dataList, gr_n, gr_p);
    }
    else {
        logError("watchlist", QString("Symbol %1 was not added to the watchlist").arg(symbol));
    }
}

void CentaurApp::onTickerUpdate(const QString &symbol, const QString &source, quint64 receivedTime, double price) noexcept
{
    const auto ms        = static_cast<qint64>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
    const qint64 latency = static_cast<qint64>(receivedTime) > ms ? 0ll : ms - static_cast<qint64>(receivedTime);

    ui()->watchListWidget->updatePriceAndLatency(symbol, source, price, latency);

    // Determinate if the OrderBook dialog is being shown
    QString orderBookObjectName = QString("%1%2orderbook").arg(source, symbol);
    auto *orderbookDlg          = this->findChild<OrderbookDialog *>(orderBookObjectName);
    if (orderbookDlg != nullptr)
        orderbookDlg->setPrice(price);

    QString depthCharDepthObjectName = QString("%1%2depth").arg(source, symbol);
    auto *depthDlg                   = this->findChild<DepthChartDialog *>(depthCharDepthObjectName);
    if (depthDlg != nullptr)
        depthDlg->setPrice(price);
}

void CentaurApp::onRemoveWatchList(const QString &itemSource, const QString &itemSymbol) noexcept
{
    logTrace("watchlist", "CentaurApp::onRemoveWatchList");
    // Retrieve the IExchange from the row based on the 5 column, which has the PluginUUID Source

    auto interfaceIter = _impl->exchangeList.find(uuid(itemSource.toStdString(), false));
    if (interfaceIter == _impl->exchangeList.end()) {
        QString message = QString(tr("Failed to locate the symbol interface."));
        logError("wlRemove", message);
        QMessageBox box;
        box.setText(tr("Could remove the symbol"));
        box.setInformativeText(message);
        box.setIcon(QMessageBox::Critical);
        box.exec();
        return;
    }

    // Call the plugin to inform that it must not send data of the symbol anymore
    auto &exchInfo = interfaceIter->second;

    exchInfo.exchange->removeSymbolFromWatchlist(itemSymbol);

    // Remove the row
    ui()->watchListWidget->removeItem(itemSymbol, itemSource);

    { // TODO: find a better way to remove from the squarify
        QLinearGradient gr_p({ 0, 0 }, { 1, 1 });
        gr_p.setCoordinateMode(QGradient::CoordinateMode::ObjectMode);
        gr_p.setColorAt(0, QColor(1, 166, 7));
        gr_p.setColorAt(0.5, QColor(26, 175, 32));
        gr_p.setColorAt(1, QColor(1, 133, 6));

        QLinearGradient gr_n({ 0, 0 }, { 1, 1 });
        gr_n.setCoordinateMode(QGradient::CoordinateMode::ObjectMode);
        gr_n.setColorAt(0, QColor(167, 29, 7));
        gr_n.setColorAt(0.5, QColor(176, 52, 32));
        gr_n.setColorAt(1, QColor(134, 23, 6));

        auto dataList = exchInfo.exchange->getWatchlist24hrPriceChange();

        std::transform(dataList.begin(), dataList.end(), dataList.begin(),
            [&exchInfo, &ui = _impl->ui](const std::tuple<qreal, qreal, QString> &data) -> std::tuple<qreal, qreal, QString> {
                ui->watchListWidget->updateDifference(std::get<2>(data), exchInfo.exchange->getPluginUUID().to_qstring(false), std::get<1>(data));
                return { std::get<0>(data),
                    std::get<1>(data),
                    QString(R"(<font size="12"><b>%1</b></font><br><font size="11"><b>$ %2</b><br>%3 %</font>)")
                        .arg(std::get<2>(data), QLocale(QLocale::English).toString(std::get<0>(data), 'f', 2))
                        .arg(std::get<1>(data), 'f', '2') };
            });

        ui()->widget->setData(dataList, gr_n, gr_p);
    }

    logInfo("watchlist", QString(tr("%1 was removed from the UI list")).arg(itemSymbol));
    dal::DataAccess::deleteFavoritesSymbol(itemSymbol, itemSource);
}

void CentaurApp::onSetWatchlistSelection(const QString &source, const QString &symbol) noexcept
{
    std::pair<QString, QString> selection = { source, symbol };

    if (_impl->currentWatchListSelection == selection)
        return;

    auto itemIter = _impl->exchangeList.find(uuid(source.toStdString(), false));
    if (itemIter == _impl->exchangeList.end()) {
        logError("wlOrderbookSend", QString("Watchlist item for the symbol %1 was not found").arg(symbol));
        return;
    }

    const auto &exchInfo = itemIter->second;

    plotSevenDaysChart(symbol, exchInfo.exchange->get7dayData(symbol));
    _impl->currentWatchListSelection = selection;
}

void CentaurApp::plotSevenDaysChart(const QString &symbol, const QList<std::pair<quint64, qreal>> &values) noexcept
{
    // Prepare the data
    if (values.size() < 7) {
        logWarn("7plot", "items less than 7");
        return;
    }

    if (values.size() > 7) {
        logWarn("7plot", "list is larger than 7; data will be truncated");
    }

    _impl->last7SevenLowSeries->clear();
    _impl->last7SevenUpSeries->clear();

    const auto yMinPrice = std::min_element(values.begin(), values.end(),
        [](const std::pair<quint64, qreal> &p1, const std::pair<quint64, qreal> &p2) {
            return p1.second < p2.second;
        })->second;
    const auto yMaxPrice = std::max_element(values.begin(), values.end(),
        [](const std::pair<quint64, qreal> &p1, const std::pair<quint64, qreal> &p2) {
            return p1.second < p2.second;
        })->second;

    const auto yMinTime = std::min_element(values.begin(), values.end(),
        [](const std::pair<quint64, qreal> &p1, const std::pair<quint64, qreal> &p2) {
            return p1.first < p2.first;
        })->first;
    const auto yMaxTime = std::max_element(values.begin(), values.end(),
        [](const std::pair<quint64, qreal> &p1, const std::pair<quint64, qreal> &p2) {
            return p1.first < p2.first;
        })->first;

    // Leave a space in the Y axis
    const auto minPos = yMinPrice - ((yMaxPrice - yMinPrice) / 2);

    qreal averageChange = 0.0;
    for (qsizetype i = values.size() - 7; i < values.size(); ++i) // If list size > 7 the -7 will truncate it to only the last 7
    {
        const auto &[time, price] = values[i];

        *_impl->last7SevenLowSeries << QPointF { static_cast<qreal>(time), minPos };
        *_impl->last7SevenUpSeries << QPointF { static_cast<qreal>(time), price };

        if (i > 0) {
            averageChange += price - values[i - 1].second;
        }
    }

    averageChange /= 6;

    if (averageChange > 0) {
        QLinearGradient gr_p({ 0.5, 0 }, { 0.5, 1 });
        gr_p.setCoordinateMode(QGradient::CoordinateMode::ObjectMode);
        gr_p.setColorAt(0, QColor(1, 166, 7, 255));
        gr_p.setColorAt(.8, QColor(1, 133, 6, 128));
        _impl->last7SevenAreaSeries->setBrush(gr_p);
    }
    else {
        QLinearGradient gr_n({ 0.5, 0 }, { 0.5, 1 });
        gr_n.setCoordinateMode(QGradient::CoordinateMode::ObjectMode);
        gr_n.setColorAt(0, QColor(167, 29, 7, 255));
        gr_n.setColorAt(.8, QColor(167, 29, 7, 128));
        _impl->last7SevenAreaSeries->setBrush(gr_n);
    }

    _impl->last7Chart->setTitle(QString(tr("%1 7-days")).arg(symbol));

    auto *xAxis = qobject_cast<QDateTimeAxis *>(_impl->last7Chart->axes(Qt::Horizontal).first());
    xAxis->setRange(
        QDateTime::fromMSecsSinceEpoch(static_cast<qint64>(yMinTime)),
        QDateTime::fromMSecsSinceEpoch(static_cast<qint64>(yMaxTime)));
    _impl->last7Chart->axes(Qt::Vertical).first()->setRange(minPos, yMaxPrice);
}

void CentaurApp::onShowPlugins() noexcept
{
    QGraphicsBlurEffect blur;

    SettingsDialog dlg(this);

    dlg.setPage(SettingsDialog::Pages::Plugins);

    dlg.exec();
}

void CentaurApp::onShowLogDialog() noexcept
{
    // Just show the dialog
    if (!_impl->logDialog->isVisible())
        _impl->logDialog->show();
}

void CentaurApp::onShowSettings() noexcept
{
    QGraphicsBlurEffect blur;
    setGraphicsEffect(&blur);

    SettingsDialog dlg(this);

    if (dlg.exec() == QDialog::Accepted) {
        updateUserInformationStatus();
    }
}

void CentaurApp::updateUserInformationStatus() noexcept
{
    static constexpr const int g_radius = 24;
    if (!g_globals->session.image.isNull()) {
        QPixmap displayedImage = QPixmap::fromImage(g_globals->session.image.scaled({ 48, 48 }));
        QBitmap map(48, 48);
        map.fill(Qt::color0);
        QPainter painter(&map);
        painter.setBrush(Qt::color1);
        painter.setRenderHint(QPainter::RenderHint::Antialiasing);
        painter.drawRoundedRect(0, 0, 48, 48, g_radius, g_radius);
        displayedImage.setMask(map);

        ui()->userImage->setPixmap(displayedImage);
    }

    const QString userDataString = QString("%1 (%2)").arg(g_globals->session.display, g_globals->session.email);
    ui()->userData->setText(userDataString);
}

void CentaurApp::onViewCandleChart(const QString &symbol, const QString &source, cen::plugin::TimeFrame tf) noexcept
{
    const QString dlgName = QString("chart_%1_%2_%3").arg(symbol, source).arg(static_cast<int>(tf));

    auto *dlg = this->findChild<CandleChartDialog *>(dlgName);
    if (dlg != nullptr) {
        dlg->show();
        dlg->setWindowState(Qt::WindowState::WindowActive);
    }
    else {
        auto *newDlg = new CandleChartDialog(tf, symbol, source, this);

        newDlg->setObjectName(dlgName);
        connect(newDlg, &CandleChartDialog::closeButtonPressed, this, [&, newDlg]() {
            newDlg->hide();
            delete newDlg;
        });

        newDlg->show();
    }
}

void CentaurApp::onToggleSidePanel() noexcept
{
    static constexpr int _defaultWidth   = 40;
    static constexpr int _defaultHeight  = 20;
    static constexpr int _maxButtonWidth = 16777215;
    static constexpr int _minButtonWidth = 35;
    static constexpr int _maxFrameWidth  = 250;
    static constexpr int _minFrameWidth  = 45;

    const std::array<QToolButton *, 3> sidePanelButtons {
        ui()->viewDashboardButton,
        ui()->viewFavoritesButton,
        ui()->viewExchangesButton
    };

    auto loadIcon = [](const QString &url) {
        QIcon icon;
        icon.addFile(url, QSize(), QIcon::Normal, QIcon::On);
        return icon;
    };

    auto updateButtonsHide = [](QToolButton *button) -> void {
        button->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonIconOnly);
        button->setMaximumWidth(_minButtonWidth);
    };

    auto updateButtonsShow = [](QToolButton *button) -> void {
        button->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
        button->setMaximumWidth(_maxButtonWidth);
    };

    if (m_sidePanelContracted) {
        // Show Panel
        ui()->sidePanelStatusFrameSpacerLeft->changeSize(_defaultWidth, _defaultHeight, QSizePolicy::Expanding, QSizePolicy::Minimum);
        ui()->sidePanelStatusFrameSpacerRight->changeSize(0, _defaultHeight, QSizePolicy::Fixed, QSizePolicy::Minimum);
        ui()->sidePanelViewButton->setIcon(loadIcon(QString::fromUtf8(":/svg/panel/menu-open")));
        std::for_each(sidePanelButtons.begin(), sidePanelButtons.end(), updateButtonsShow);
    }
    else {
        // Hide panel
        ui()->sidePanelStatusFrameSpacerLeft->changeSize(0, _defaultHeight, QSizePolicy::Fixed, QSizePolicy::Minimum);
        ui()->sidePanelStatusFrameSpacerRight->changeSize(_defaultWidth, _defaultHeight, QSizePolicy::Expanding, QSizePolicy::Minimum);
        ui()->sidePanelViewButton->setIcon(loadIcon(QString::fromUtf8(":/svg/panel/menu")));
        std::for_each(sidePanelButtons.begin(), sidePanelButtons.end(), updateButtonsHide);
    }

    ui()->sidePanelToggleLayout->invalidate();

    auto *iTheme = cuiTheme();
    using namespace CENTAUR_THEME_INTERFACE_NAMESPACE;
    const auto *animation = [&panelIsContracted = m_sidePanelContracted, &iTheme]() -> ThemeAnimation * {
        if (iTheme == nullptr)
            return nullptr;
        if (!panelIsContracted)
            return iTheme->uiElements().sideFrameInformation.hidePanelAnimation;
        return iTheme->uiElements().sideFrameInformation.showPanelAnimation;
    }();

    if (!m_useAnimations || animation == nullptr) {
        QSize currentPanelFrameSize = ui()->sideBarLeftFrame->maximumSize();
        if (!m_sidePanelContracted) {
            currentPanelFrameSize.setWidth(_minFrameWidth);
        }
        else {
            currentPanelFrameSize.setWidth(_maxFrameWidth);
        }
        ui()->sideBarLeftFrame->setMaximumSize(currentPanelFrameSize);
    }
    else {

        auto *maxSizeAnimation = createObject<QPropertyAnimation>(ui()->sideBarLeftFrame, "maximumSize", this);

        const auto originSize = ui()->sideBarLeftFrame->maximumSize();
        maxSizeAnimation->setDuration(animation->duration);
        maxSizeAnimation->setEasingCurve(animation->easingCurve);
        maxSizeAnimation->setStartValue(originSize);
        maxSizeAnimation->setEndValue(
            m_sidePanelContracted
                ? QSize(_maxFrameWidth, originSize.height())
                : QSize(_minFrameWidth, originSize.height()));
        maxSizeAnimation->start(QAbstractAnimation::DeleteWhenStopped);
    }

    m_sidePanelContracted = !m_sidePanelContracted;
}

END_CENTAUR_NAMESPACE
