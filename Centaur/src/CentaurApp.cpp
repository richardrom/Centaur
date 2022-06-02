/////////////////////////////////////////////////////////////////////////////////////
//
// Created by Ricardo Romero on 06/12/21.
// Copyright (c) 2021 Ricardo Romero.  All rights reserved.
//

#include "CentaurApp.hpp"
#include "HTMLDelegate.hpp"
#include "PluginsDialog.hpp"
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlField>
#include <QSqlQuery>
#include <QSqlRecord>

CENTAUR_NAMESPACE::CentaurApp *CENTAUR_NAMESPACE::g_app = nullptr;
namespace CENTAUR_NAMESPACE
{
    class FavoritesDBManager final
    {
    public:
        inline FavoritesDBManager()
        {
            m_insert.prepare("INSERT INTO favorites(symbol,plugin) VALUES(:symbol, :plugin);");
            m_delete.prepare("DELETE FROM favorites WHERE symbol = :symbol AND plugin = :plugin;");
            m_select.prepare("SELECT symbol,plugin FROM favorites;");
        }
        ~FavoritesDBManager() = default;

    public:
        inline void add(const QString &symbol, const QString &uuid)
        {
            m_insert.bindValue(":symbol", symbol);
            m_insert.bindValue(":plugin", uuid);

            if (!m_insert.exec())
            {
                logError("app", QString(LS("error-fav-db-insert").arg(m_insert.lastError().text())));
            }
        }

        inline QList<QPair<QString, QString>> selectAll()
        {
            QList<QPair<QString, QString>> data;
            if (!m_select.exec())
            {
                logError("app", QString(LS("error-fav-db-select").arg(m_select.lastError().text())));
                return data;
            }

            while (m_select.next())
            {
                const QSqlRecord currentRecord = m_select.record();
                QSqlField genre                = currentRecord.field("plugin");
                data.emplaceBack(currentRecord.field("symbol").value().toString(), currentRecord.field("plugin").value().toString());
            }

            return data;
        }

        inline void del(const QString &symbol, const QString &uuid)
        {
            m_delete.bindValue(":symbol", symbol);
            m_delete.bindValue(":plugin", uuid);

            if (!m_delete.exec())
            {
                logError("app", QString(LS("error-fav-db-delete").arg(m_delete.lastError().text())));
            }
        }

    private:
        QSqlQuery m_insert;
        QSqlQuery m_delete;
        QSqlQuery m_select;
        QSqlDatabase m_db;
    };
} // namespace CENTAUR_NAMESPACE

CENTAUR_NAMESPACE::CentaurApp::CentaurApp(QWidget *parent) :
    QMainWindow(parent),
    m_ui { std::make_unique<Ui::CentaurApp>() }
{
    START_TIME(initializationTimeStart);

    g_app     = this;
    g_globals = new Globals;

    m_ui->setupUi(this);
    installEventFilter(this);

#ifdef Q_OS_MAC
    CFURLRef appUrlRef       = CFBundleCopyBundleURL(CFBundleGetMainBundle());
    CFStringRef macPath      = CFURLCopyFileSystemPath(appUrlRef,
             kCFURLPOSIXPathStyle);
    g_globals->paths.appPath = CFStringGetCStringPtr(macPath, CFStringGetSystemEncoding());
    CFRelease(appUrlRef);
    CFRelease(macPath);
#else

#endif /*Q_OS_MAC*/

    g_globals->paths.installPath = "/Volumes/RicardoESSD/Projects/Centaur/local";
    g_globals->paths.pluginsPath = g_globals->paths.installPath + "/Plugin";
    g_globals->paths.resPath     = g_globals->paths.installPath + "/Resources";
    
    // Start logging service
    startLoggingService();

    // Load External XML Configuration Data
    loadConfigurationData();

    // Init the database internal file
    initializeDatabaseServices();

    // Restore file
    loadInterfaceState();

    // Start the interface
    initializeInterface();

    // Load plugins
    loadPlugins();

    // Load the favorites list. Since all plugins must be loaded
    loadFavoritesWatchList();

    END_TIME_SEC(initializationTimeStart, initializationTimeEnd, initializationTime);
    logInfo("app", QString(LS("trace-initialize-time")).arg(initializationTime.count(), 0, 'f', 4));
}
CENTAUR_NAMESPACE::CentaurApp::~CentaurApp()
{
    for (auto &pli : m_configurationInterface)
    {
        delete pli.second;
    }
    if (m_sqlFavorites != nullptr)
    {
        delete m_sqlFavorites;
        m_sqlFavorites = nullptr;
    }
    if (g_logger != nullptr)
    {
        g_logger->terminate();
        m_loggerThread->join();
    }

    delete g_globals;
}

bool CENTAUR_NAMESPACE::CentaurApp::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == this && event->type() == QEvent::Close)
        saveInterfaceState();

    return QObject::eventFilter(obj, event);
}

void cen::CentaurApp::initializeDatabaseServices() noexcept
{
    auto db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(g_globals->paths.resPath + "/Local/centaur.db");

    if (!db.open())
    {
        logFatal("app", LS("fatal-centaur-db"));
        QMessageBox::critical(this, LS("error-error"), LS("fatal-centaur-db"));
        exit(EXIT_FAILURE);
    }

    m_sqlFavorites = new FavoritesDBManager;
}

void CENTAUR_NAMESPACE::CentaurApp::initializeInterface() noexcept
{
    logTrace("app", "CENTAUR_NAMESPACE::CentaurApp::initializeInterface()");

    auto *aboutMenu         = new QMenu,
         *preferencesMenu   = new QMenu,
         *pluginsMenu       = new QMenu;
    auto *aboutAction       = new QAction(tr("About Centaur"));
    auto *preferencesAction = new QAction(tr("Preferences"));
    auto *pluginsAction     = new QAction(tr("Plugins"));

    aboutAction->setMenuRole(QAction::MenuRole::AboutQtRole);
    preferencesAction->setMenuRole(QAction::MenuRole::PreferencesRole);
    pluginsAction->setMenuRole(QAction::MenuRole::ApplicationSpecificRole);
    aboutMenu->addAction(aboutAction);
    preferencesMenu->addAction(preferencesAction);
    pluginsMenu->addAction(pluginsAction);
    m_ui->m_menuBar->addMenu(aboutMenu);
    m_ui->m_menuBar->addMenu(preferencesMenu);
    m_ui->m_menuBar->addMenu(pluginsMenu);

    connect(pluginsAction, SIGNAL(triggered()), this, SLOT(onPlugins()));

    m_ui->m_statusBar->setStyleSheet("QStatusBar::item { border: 2px; }");

    m_ui->tabSymbols->setTabText(m_ui->tabSymbols->indexOf(m_ui->tabWatchList), LS("ui-docks-watchlist"));
    // Menu actions
    connect(m_ui->m_actionTileWindows, SIGNAL(triggered()), this, SLOT(onActionTileWindowsTriggered()));
    connect(m_ui->m_actionCascadeWindows, SIGNAL(triggered()), this, SLOT(onActionCascadeWindowsTriggered()));
    connect(m_ui->m_actionAsks, SIGNAL(toggled(bool)), this, SLOT(onActionAsksToggled(bool)));
    connect(m_ui->m_actionBalances, SIGNAL(toggled(bool)), this, SLOT(onActionBalancesToggled(bool)));
    connect(m_ui->m_actionBids, SIGNAL(toggled(bool)), this, SLOT(onActionBidsToggled(bool)));
    connect(m_ui->m_actionDepth, SIGNAL(toggled(bool)), this, SLOT(onActionDepthToggled(bool)));
    connect(m_ui->m_actionLogging, SIGNAL(toggled(bool)), this, SLOT(onActionLoggingToggled(bool)));
    connect(m_ui->m_actionSymbols, SIGNAL(toggled(bool)), this, SLOT(onActionSymbolsToggled(bool)));

    // Remove the first index
    m_ui->dockSymbols->setWindowTitle(LS("ui-docks-symbols"));
    m_ui->tabSymbols->removeTab(1);

    // Show or hide the docking windows according to the store status of the Gm_ui
    m_ui->m_actionSymbols->setChecked(!m_ui->dockSymbols->isHidden());
    m_ui->m_actionLogging->setChecked(!m_ui->m_dockLogging->isHidden());
    m_ui->m_actionBalances->setChecked(!m_ui->m_dockBalances->isHidden());
    m_ui->m_actionBids->setChecked(!m_ui->dockOrderbookBids->isHidden());
    m_ui->m_actionAsks->setChecked(!m_ui->dockOrderbookAsks->isHidden());
    m_ui->m_actionDepth->setChecked(!m_ui->m_dockDepth->isHidden());

    // Init the watchlist QLineEdit
    m_ui->editWatchListFilter->setPlaceholderText(LS("ui-docks-search"));
    m_ui->editWatchListFilter->addAction(g_globals->icons.searchIcon, QLineEdit::LeadingPosition);
    m_watchlistItemModel = new QStandardItemModel(0, 4, m_ui->tabWatchList);
    auto sortProxyModel  = new QSortFilterProxyModel(m_ui->tabWatchList);
    sortProxyModel->setSourceModel(m_watchlistItemModel);
    m_ui->listWatchList->setModel(sortProxyModel);
    m_ui->listWatchList->setRemove();
    m_ui->listWatchList->allowClickMessages();
    connect(m_ui->listWatchList, &CENTAUR_NAMESPACE::CenListCtrl::snRemoveWatchList, this, &CentaurApp::onRemoveWatchList);
    connect(m_ui->listWatchList, &CENTAUR_NAMESPACE::CenListCtrl::snSetSelection, this, &CentaurApp::onSetWatchlistSelection);
    connect(m_ui->listWatchList, &CENTAUR_NAMESPACE::CenListCtrl::snRemoveSelection, this, &CentaurApp::onWatchlistRemoveSelection);

    sortProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    connect(m_ui->editWatchListFilter, &QLineEdit::textChanged, sortProxyModel, &QSortFilterProxyModel::setFilterFixedString);

    m_ui->listWatchList->verticalHeader()->setFont(g_globals->fonts.symbolsDock.headerFont);
    m_ui->listWatchList->sortByColumn(0, Qt::AscendingOrder);
    m_ui->listWatchList->setEditTriggers(QAbstractItemView::NoEditTriggers);

    m_watchlistItemModel->setHorizontalHeaderLabels(
        { LS("ui-docks-symbol"),
            LS("ui-docks-price"),
            LS("ui-docks-source"),
            LS("ui-docks-latency"),
            "UUID" });

    m_watchlistItemModel->horizontalHeaderItem(0)->setFont(g_globals->fonts.symbolsDock.headerFont);
    m_watchlistItemModel->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_watchlistItemModel->horizontalHeaderItem(1)->setFont(g_globals->fonts.symbolsDock.headerFont);
    m_watchlistItemModel->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_watchlistItemModel->horizontalHeaderItem(2)->setFont(g_globals->fonts.symbolsDock.headerFont);
    m_watchlistItemModel->horizontalHeaderItem(2)->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_watchlistItemModel->horizontalHeaderItem(3)->setFont(g_globals->fonts.symbolsDock.headerFont);
    m_watchlistItemModel->horizontalHeaderItem(3)->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    // This column, although hidden, will be used to find uuid source of all watchlist items
    m_watchlistItemModel->horizontalHeaderItem(4)->setFont(g_globals->fonts.symbolsDock.headerFont);
    m_watchlistItemModel->horizontalHeaderItem(4)->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_ui->listWatchList->setColumnHidden(4, true);

    m_ui->listWatchList->setColumnWidth(0, m_uiState.wlcols.symbol);
    m_ui->listWatchList->setColumnWidth(1, m_uiState.wlcols.price);
    m_ui->listWatchList->setColumnWidth(2, m_uiState.wlcols.sender);
    m_ui->listWatchList->setColumnWidth(3, m_uiState.wlcols.latency);

    // Balances Tree
    m_ui->m_ctrlBalances->setHeaderLabels({ "Name", "Value" });
    m_ui->m_ctrlBalances->setColumnWidth(0, m_uiState.blcols.name);
    m_ui->m_ctrlBalances->setColumnWidth(1, m_uiState.blcols.value);
    m_ui->m_ctrlBalances->setIconSize(QSize(32, 32));

    // Init the logging window
    QTableWidget *logger = m_ui->m_logs;
    logger->setHorizontalHeaderLabels({ "Date", "User", "Session", "Type", "Source", "Message" });
    logger->horizontalHeaderItem(0)->setFont(QFont("Arial", 10));
    logger->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft);
    logger->horizontalHeaderItem(1)->setFont(QFont("Arial", 10));
    logger->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignCenter);
    logger->horizontalHeaderItem(2)->setFont(QFont("Arial", 10));
    logger->horizontalHeaderItem(2)->setTextAlignment(Qt::AlignCenter);
    logger->horizontalHeaderItem(3)->setFont(QFont("Arial", 10));
    logger->horizontalHeaderItem(3)->setTextAlignment(Qt::AlignCenter);
    logger->horizontalHeaderItem(4)->setFont(QFont("Arial", 10));
    logger->horizontalHeaderItem(4)->setTextAlignment(Qt::AlignCenter);
    logger->horizontalHeaderItem(5)->setFont(QFont("Arial", 10));
    logger->horizontalHeaderItem(5)->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    logger->setColumnWidth(0, m_uiState.lgcols.date);
    logger->setColumnWidth(1, m_uiState.lgcols.user);
    logger->setColumnWidth(2, m_uiState.lgcols.session);
    logger->setColumnWidth(3, m_uiState.lgcols.type);
    logger->setColumnWidth(4, m_uiState.lgcols.source);
    logger->setColumnWidth(5, m_uiState.lgcols.message);
    logger->setItemDelegateForColumn(5, new HTMLDelegate);

    // Orderbook asks
    //    QString styleSheet = "::section {" // "QHeaderView::section {"
    //                        "font-family: arial;"
    //                       "font-size: 10px; }";

    m_ui->asksTable->setHorizontalHeaderLabels({ "Price", "Amount", "Total" });
    m_ui->asksTable->horizontalHeaderItem(0)->setFont(QFont("Arial", 10));
    m_ui->asksTable->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_ui->asksTable->horizontalHeaderItem(1)->setFont(QFont("Arial", 10));
    m_ui->asksTable->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignRight);
    m_ui->asksTable->horizontalHeaderItem(2)->setFont(QFont("Arial", 10));
    m_ui->asksTable->horizontalHeaderItem(2)->setTextAlignment(Qt::AlignRight);
    m_ui->asksTable->setColumnWidth(0, m_uiState.askscols.price);
    m_ui->asksTable->setColumnWidth(1, m_uiState.askscols.amount);
    m_ui->asksTable->setColumnWidth(2, m_uiState.askscols.total);
    //   m_ui->asksTable->verticalHeader()->setStyleSheet(styleSheet);

    // Orderbook bids
    m_ui->bidsTable->setHorizontalHeaderLabels({ "Price", "Amount", "Total" });
    m_ui->bidsTable->horizontalHeaderItem(0)->setFont(QFont("Arial", 10));
    m_ui->bidsTable->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft);
    m_ui->bidsTable->horizontalHeaderItem(1)->setFont(QFont("Arial", 10));
    m_ui->bidsTable->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignRight);
    m_ui->bidsTable->horizontalHeaderItem(2)->setFont(QFont("Arial", 10));
    m_ui->bidsTable->horizontalHeaderItem(2)->setTextAlignment(Qt::AlignRight);
    m_ui->bidsTable->setColumnWidth(0, m_uiState.bidscols.price);
    m_ui->bidsTable->setColumnWidth(1, m_uiState.bidscols.amount);
    m_ui->bidsTable->setColumnWidth(2, m_uiState.bidscols.total);
    //  m_ui->bidsTable->verticalHeader()->setStyleSheet(styleSheet);

    /*
    m_ui->m_asksDepth->setAxisVisible(QwtPlot::Axis::yLeft, false);
    m_ui->m_asksDepth->setAxisVisible(QwtPlot::Axis::yRight);

    m_ui->m_bidsDepth->canvas()->setStyleSheet("border: 0px;");
    m_ui->m_asksDepth->canvas()->setStyleSheet("border: 0px;");

    QColor green(0x2f, 0xf9, 0x24), greenLow = green;
    greenLow.setAlpha(20);
    QBrush greenBrush(greenLow);
    m_plotBidsDepth = new QwtPlotCurve("Bids depth");
    m_plotBidsDepth->setPen(green, 1.0);
    m_plotBidsDepth->setRenderThreadCount(2);
    m_plotBidsDepth->setBrush(greenBrush);
    m_plotBidsDepth->setRenderHint(QwtPlotItem::RenderAntialiased);
    m_plotBidsDepth->attach(m_ui->m_bidsDepth);
    m_ui->m_bidsDepth->setAxisScaleEngine(QwtPlot::Axis::yLeft, new QwtLinearScaleEngine);
    m_ui->m_bidsDepth->axisScaleEngine(QwtPlot::Axis::yRight)->setAttribute(QwtScaleEngine::Floating, true);

    m_ui->m_bidsDepth->setAxisScaleEngine(QwtPlot::Axis::xBottom, new QwtLinearScaleEngine);
    m_ui->m_bidsDepth->axisScaleEngine(QwtPlot::Axis::xBottom)->setAttribute(QwtScaleEngine::Floating, true);
    m_ui->m_bidsDepth->axisScaleEngine(QwtPlot::Axis::xBottom)->setAttribute(QwtScaleEngine::Inverted, true);

    QColor red(0xd2, 0x10, 0x04), redLow = red;
    redLow.setAlpha(20);
    QBrush redBrush(redLow);
    m_plotAsksDepth = new QwtPlotCurve("Asks depth");
    m_plotAsksDepth->setPen(red, 1.0);
    m_plotAsksDepth->setBrush(redBrush);
    m_plotAsksDepth->setRenderThreadCount(2);
    m_plotAsksDepth->setRenderHint(QwtPlotItem::RenderAntialiased);
    m_plotAsksDepth->attach(m_ui->m_asksDepth);
    m_ui->m_asksDepth->setAxisScaleEngine(QwtPlot::Axis::yRight, new QwtLinearScaleEngine);
    m_ui->m_asksDepth->axisScaleEngine(QwtPlot::Axis::yRight)->setAttribute(QwtScaleEngine::Floating, true);*/
}

void CENTAUR_NAMESPACE::CentaurApp::saveInterfaceState() noexcept
{
    logTrace("app", "CentaurApp::saveInterfaceState()");

    QSettings settings("CentaurProject", "Centaur");

    settings.beginGroup("mainWindow");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("state", saveState());
    settings.endGroup();

    settings.beginGroup("watchlistListState");
    settings.setValue("c0", m_ui->listWatchList->columnWidth(0));
    settings.setValue("c1", m_ui->listWatchList->columnWidth(1));
    settings.setValue("c2", m_ui->listWatchList->columnWidth(2));
    settings.setValue("c3", m_ui->listWatchList->columnWidth(3));
    settings.endGroup();

    settings.beginGroup("loggingListState");
    settings.setValue("c0", m_ui->m_logs->columnWidth(0));
    settings.setValue("c1", m_ui->m_logs->columnWidth(1));
    settings.setValue("c2", m_ui->m_logs->columnWidth(2));
    settings.setValue("c3", m_ui->m_logs->columnWidth(3));
    settings.setValue("c4", m_ui->m_logs->columnWidth(4));
    settings.setValue("c5", m_ui->m_logs->columnWidth(5));
    settings.endGroup();

    settings.beginGroup("BalancesTreeState");
    settings.setValue("c0", m_ui->m_ctrlBalances->columnWidth(0));
    settings.setValue("c1", m_ui->m_ctrlBalances->columnWidth(1));
    settings.endGroup();

    settings.beginGroup("OrderbookAsksState");
    settings.setValue("c0", m_ui->asksTable->columnWidth(0));
    settings.setValue("c1", m_ui->asksTable->columnWidth(1));
    settings.setValue("c2", m_ui->asksTable->columnWidth(2));
    settings.endGroup();

    settings.beginGroup("OrderbookBidsState");
    settings.setValue("c0", m_ui->bidsTable->columnWidth(0));
    settings.setValue("c1", m_ui->bidsTable->columnWidth(1));
    settings.setValue("c2", m_ui->bidsTable->columnWidth(2));
    settings.endGroup();

    logInfo("app", "UI state saved");
}

void CENTAUR_NAMESPACE::CentaurApp::loadInterfaceState() noexcept
{
    logTrace("app", "CentaurApp::loadInterfaceState()");

    QSettings settings("CentaurProject", "Centaur");

    settings.beginGroup("mainWindow");
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("state").toByteArray());
    settings.endGroup();

    settings.beginGroup("watchlistListState");
    m_uiState.wlcols.symbol  = settings.value("c0", m_uiState.wlcols.symbol).toInt();
    m_uiState.wlcols.price   = settings.value("c1", m_uiState.wlcols.price).toInt();
    m_uiState.wlcols.sender  = settings.value("c2", m_uiState.wlcols.sender).toInt();
    m_uiState.wlcols.latency = settings.value("c3", m_uiState.wlcols.latency).toInt();
    settings.endGroup();

    settings.beginGroup("loggingListState");
    m_uiState.lgcols.date    = settings.value("c0", m_uiState.lgcols.date).toInt();
    m_uiState.lgcols.session = settings.value("c1", m_uiState.lgcols.session).toInt();
    m_uiState.lgcols.user    = settings.value("c2", m_uiState.lgcols.user).toInt();
    m_uiState.lgcols.type    = settings.value("c3", m_uiState.lgcols.type).toInt();
    m_uiState.lgcols.source  = settings.value("c4", m_uiState.lgcols.source).toInt();
    m_uiState.lgcols.message = settings.value("c5", m_uiState.lgcols.message).toInt();
    settings.endGroup();

    settings.beginGroup("BalancesTreeState");
    m_uiState.blcols.name  = settings.value("c0", m_uiState.blcols.name).toInt();
    m_uiState.blcols.value = settings.value("c1", m_uiState.blcols.value).toInt();
    settings.endGroup();

    settings.beginGroup("OrderbookAsksState");
    m_uiState.askscols.price  = settings.value("c0", m_uiState.askscols.price).toInt();
    m_uiState.askscols.amount = settings.value("c1", m_uiState.askscols.amount).toInt();
    m_uiState.askscols.total  = settings.value("c2", m_uiState.askscols.total).toInt();
    settings.endGroup();

    settings.beginGroup("OrderbookBidsState");
    m_uiState.bidscols.price  = settings.value("c0", m_uiState.bidscols.price).toInt();
    m_uiState.bidscols.amount = settings.value("c1", m_uiState.bidscols.amount).toInt();
    m_uiState.bidscols.total  = settings.value("c2", m_uiState.bidscols.total).toInt();
    settings.endGroup();

    logInfo("app", "UI state loaded");
}

void cen::CentaurApp::loadFavoritesWatchList() noexcept
{
    auto data = m_sqlFavorites->selectAll();

    if (data.isEmpty())
    {
        logInfo("app", LS("info-fav-db-empty"));
        return;
    }

    for (const auto &[sym, plid] : data)
    {
        if (sym.isEmpty())
        {
            logWarn("app", LS("warning-fav-symbol-empty"));
            continue;
        }

        if (plid.isEmpty())
        {
            logWarn("app", LS("warning-fav-uuid-empty"));
            continue;
        }

        // Insert the element
        onAddToWatchList(sym, plid, false);
    }
}

void CENTAUR_NAMESPACE::CentaurApp::startLoggingService() noexcept
{
    g_logger = new CentaurLogger;
    // Init the logger
    m_loggerThread = std::make_unique<std::thread>(&CENTAUR_NAMESPACE::CentaurLogger::run, g_logger);
    try
    {
        g_logger->setApplication(this);
        g_logger->setUser("root");
        g_logger->setSession(0);
        logInfo("app", QString("TraderSys ") + QString(CENTAUR_NAMESPACE::CentaurVersionString));
    } catch (const std::runtime_error &ex)
    {
        QMessageBox::critical(this,
            tr("Fatal error"),
            ex.what());
        QApplication::quit();
    }
}

void CENTAUR_NAMESPACE::CentaurApp::onActionTileWindowsTriggered()
{
    m_ui->m_mdiArea->tileSubWindows();
}

void CENTAUR_NAMESPACE::CentaurApp::onActionCascadeWindowsTriggered()
{
    m_ui->m_mdiArea->cascadeSubWindows();
}

void CENTAUR_NAMESPACE::CentaurApp::onActionSymbolsToggled(bool status)
{
    status ? m_ui->dockSymbols->show() : m_ui->dockSymbols->hide();
}

void CENTAUR_NAMESPACE::CentaurApp::onActionLoggingToggled(bool status)
{
    status ? m_ui->m_dockLogging->show() : m_ui->m_dockLogging->hide();
}
void CENTAUR_NAMESPACE::CentaurApp::onActionBalancesToggled(bool status)
{
    status ? m_ui->m_dockBalances->show() : m_ui->m_dockBalances->hide();
}
void CENTAUR_NAMESPACE::CentaurApp::onActionAsksToggled(bool status)
{
    status ? m_ui->dockOrderbookAsks->show() : m_ui->dockOrderbookAsks->hide();
}
void CENTAUR_NAMESPACE::CentaurApp::onActionBidsToggled(bool status)
{
    status ? m_ui->dockOrderbookBids->show() : m_ui->dockOrderbookBids->hide();
}
void CENTAUR_NAMESPACE::CentaurApp::onActionDepthToggled(bool status)
{
    status ? m_ui->m_dockDepth->show() : m_ui->m_dockDepth->hide();
}

void CENTAUR_NAMESPACE::CentaurApp::onAddToWatchList(const QString &symbol, const QString &sender, bool addToDatabase) noexcept
{
    logTrace("watchlist", "CentaurApp::onAddToWatchList()");

    auto interface = m_exchangeList.find(sender);

    if (interface == m_exchangeList.end())
    {
        logError("watchlist", QString(tr("The sender '%1' is not registered.")).arg(sender));
        return;
    }

    auto watchInterface = interface->second.exchange;

    // Generate a unique-id
    const int id = ++m_sessionIds;

    if (watchInterface->addSymbol(symbol, id))
    {
        auto itemSymbol  = new QStandardItem(symbol);
        auto itemPrice   = new QStandardItem("$ 0.00");
        auto itemSource  = new QStandardItem(interface->second.listName);
        auto itemLatency = new QStandardItem("0");
        auto itemUUID    = new QStandardItem(interface->second.uuid.to_string().c_str());
        int curRow       = m_watchlistItemModel->rowCount();
        itemSymbol->setFont(g_globals->fonts.symbolsDock.tableFont);
        itemSymbol->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        itemPrice->setFont(g_globals->fonts.symbolsDock.tableFont);
        itemPrice->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        itemSource->setFont(g_globals->fonts.symbolsDock.tableFont);
        itemSource->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        itemLatency->setFont(g_globals->fonts.symbolsDock.tableFont);
        itemLatency->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        m_watchlistItemModel->insertRow(curRow, { itemSymbol, itemPrice, itemSource, itemLatency, itemUUID });
        m_watchlistItems[id] = { itemSymbol, 0. };

        // Add to the database
        if (addToDatabase)
            addFavoritesWatchListDB(symbol, sender);
    }
    else
    {
        logError("watchlist", QString("Symbol %1 was not added to the watchlist").arg(symbol));
    }
}
void CENTAUR_NAMESPACE::CentaurApp::onTickerUpdate(const QString &symbol, const int &id, const quint64 &receivedTime, const double &price) noexcept
{
    // Find the item in the Watchlist items
    auto itemIter         = m_watchlistItems.find(id);
    auto &itemInformation = itemIter->second;

    if (itemIter == m_watchlistItems.end())
    {
        logError("wlTickerUpdate", QString("Watchlist item for the symbol '%1' was not found").arg(symbol));
        return;
    }

    const double previousPrice = [&]() -> double {
        double p                      = itemInformation.previousPrice;
        itemInformation.previousPrice = price; // Update the price
        return p;
    }();

    auto item         = itemInformation.listItem;
    const int itemRow = item->row();

    // Get the latency item
    auto itemLatency = m_watchlistItemModel->item(itemRow, 3);
    // Get the price item
    auto itemPrice = m_watchlistItemModel->item(itemRow, 1);
    if (price > previousPrice)
    {
        item->setIcon(g_globals->icons.upArrow);
        itemPrice->setData(QBrush(g_globals->colors.symbolsDockColors.priceUp), Qt::ForegroundRole);
    }
    else if (price < previousPrice)
    {
        item->setIcon(g_globals->icons.downArrow);
        itemPrice->setData(QBrush(g_globals->colors.symbolsDockColors.priceDown), Qt::ForegroundRole);
    }
    else
    {
        item->setIcon(g_globals->icons.downArrow);
        itemPrice->setData(QBrush(g_globals->colors.symbolsDockColors.priceNeutral), Qt::ForegroundRole);
    }
    itemPrice->setText("$ " + QLocale(QLocale::English).toString(price, 'f', 5));

    // Calculate latency
    const auto ms      = static_cast<quint64>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
    const auto latency = receivedTime > ms ? 0ull : ms - receivedTime;

    if (latency >= g_globals->params.symbolsDockParameters.latencyLowMin && latency <= g_globals->params.symbolsDockParameters.latencyLowMax)
    {
        itemLatency->setData(QBrush(g_globals->colors.symbolsDockColors.latencyLow), Qt::ForegroundRole);
    }
    else if (latency >= g_globals->params.symbolsDockParameters.latencyMediumMin && latency <= g_globals->params.symbolsDockParameters.latencyMediumMax)
    {
        itemLatency->setData(QBrush(g_globals->colors.symbolsDockColors.latencyMedium), Qt::ForegroundRole);
    }
    else if (latency >= g_globals->params.symbolsDockParameters.latencyHighMin && latency <= g_globals->params.symbolsDockParameters.latencyHighMax)
    {
        itemLatency->setData(QBrush(g_globals->colors.symbolsDockColors.latencyHigh), Qt::ForegroundRole);
    }
    itemLatency->setText(QString("%1 ms").arg(latency));
}

void CENTAUR_NAMESPACE::CentaurApp::onRemoveWatchList(const int &row) noexcept
{
    logTrace("watchlist", "CentaurApp::onRemoveWatchList");
    /// Retrieve the IExchange from the row based on the 4 column which has the PluginUUID Source
    auto index         = m_watchlistItemModel->item(row, 0);
    QString itemSymbol = index->text();
    QString itemSource = m_watchlistItemModel->item(row, 4)->text();

    auto interfaceIter = m_exchangeList.find(itemSource);
    if (interfaceIter == m_exchangeList.end())
    {
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

    exchInfo.exchange->removeSymbol(itemSymbol);

    // Remove the row
    if (!m_watchlistItemModel->removeRow(row))
        logError("watchlist", QString(tr("%1 was not removed from the UI list")).arg(itemSymbol));
    else
        logInfo("watchlist", QString(tr("%1 was removed from the UI list")).arg(itemSymbol));

    // Find the item id
    int id { -1 };
    for (auto &m_watchlistItem : m_watchlistItems)
    {
        if (m_watchlistItem.second.listItem == index)
        {
            id = m_watchlistItem.first;
            logDebug("watchlist", QString("QStandardItem found with id %1").arg(id));
            break;
        }
    }
    // Remove the data associated
    if (id >= 0)
    {
        m_watchlistItems.erase(id);
        logInfo("watchlist", QString(tr("%1 from %2 removed from the watchlist")).arg(itemSymbol, itemSource));
        // Remove from the favorites DB
        removeFavoritesWatchListDB(itemSymbol, itemSource);

        if (m_watchlistItems.empty())
        {
            // Clear all the data
            clearOrderbookListsAndDepth();
        }
    }
    else
        logError("watchlist", "could not locate the item to remove");
}

void CENTAUR_NAMESPACE::CentaurApp::onSetWatchlistSelection(const QString &source, const QString &symbol) noexcept
{
    auto itemIter = m_exchangeList.find(source);
    if (itemIter == m_exchangeList.end())
    {
        logError("wlOrderbookSend", QString("Watchlist item for the symbol %1 was not found").arg(symbol));
        return;
    }

    const auto &[curSource, curSymbol] = m_currentViewOrderbookSymbol;

    if (curSource == source && curSymbol == symbol)
    {
        logInfo("wlOrderbookSend", "Symbol data is already being received");
        return;
    }

    auto &interface = itemIter->second.exchange;
    interface->updateOrderbook(symbol);
    m_currentViewOrderbookSymbol = { source, symbol };
    m_ui->bidsSymbol->setText(QString("%1 - %2").arg(symbol, itemIter->second.listName));
    m_ui->asksSymbol->setText(QString("%1 - %2").arg(symbol, itemIter->second.listName));
}

void CENTAUR_NAMESPACE::CentaurApp::onWatchlistRemoveSelection() noexcept
{
    auto &[source, symbol] = m_currentViewOrderbookSymbol;
    if (source.isEmpty() || symbol.isEmpty())
        return;

    auto itemIter = m_exchangeList.find(source);
    if (itemIter == m_exchangeList.end())
    {
        logError("wlOrderbookStop", QString("Watchlist item for the symbol %1 was not found").arg(symbol));
        return;
    }
    auto &interface = itemIter->second.exchange;
    interface->stopOrderbook(symbol);
    clearOrderbookListsAndDepth();
}

void CENTAUR_NAMESPACE::CentaurApp::onOrderbookUpdate(const QString &source, const QString &symbol, const quint64 &receivedTime, const QMap<QString, QPair<QString, QString>> &bids, const QMap<QString, QPair<QString, QString>> &asks) noexcept
{
    const auto &[curSource, curSymbol] = m_currentViewOrderbookSymbol;

    if (curSource != source || curSymbol != symbol)
        return;

    auto insertTable = [](const QString &text, QTableWidget *ui, int row, int col, int type) {
        QTableWidgetItem *item = ui->item(row, col);
        if (item == nullptr)
        {
            item = new QTableWidgetItem(text);

            item->setFont(QFont("Arial", 10));

            switch (col)
            {
                case 1:
                    item->setTextAlignment(Qt::AlignCenter);
                    break;
                case 2:
                    item->setTextAlignment(Qt::AlignRight);
                    break;
                case 0:
                default:
                    item->setTextAlignment(Qt::AlignLeft);
                    if (type == 1)
                        item->setForeground(QBrush(QColor(0xFFA7AC)));
                    else
                        item->setForeground(QBrush(QColor(0x9CFFB4)));
                    break;
            }
            item->setTextAlignment(Qt::AlignVCenter);
            ui->setItem(row, col, item);
        }

        // Quite an expensive function
        item->setText(text);
    };

    int nRowIndex = -1;

    m_ui->asksTable->setRowCount(static_cast<int>(asks.size()));
    nRowIndex = 0;
    for (auto iter = asks.begin(); iter != asks.end(); ++iter)
    {
        insertTable(iter.key(), m_ui->asksTable, nRowIndex, 0, 1);
        insertTable(iter.value().first, m_ui->asksTable, nRowIndex, 1, 1);
        insertTable(iter.value().second, m_ui->asksTable, nRowIndex, 2, 1);
        ++nRowIndex;
    }

    m_ui->bidsTable->setRowCount(static_cast<int>(bids.size()));
    nRowIndex = 0;
    for (auto iter = bids.begin(); iter != bids.end(); ++iter)
    {
        insertTable(iter.key(), m_ui->bidsTable, nRowIndex, 0, 0);
        insertTable(iter.value().first, m_ui->bidsTable, nRowIndex, 1, 0);
        insertTable(iter.value().second, m_ui->bidsTable, nRowIndex, 2, 0);
        ++nRowIndex;
    }

    // Calculate latency
    const auto ms      = static_cast<quint64>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
    const auto latency = receivedTime > ms ? 0ull : ms - receivedTime;

    auto changeColor   = [](QLabel *ctr, const QColor &color) {
        QPalette pal = ctr->palette();
        pal.setColor(ctr->foregroundRole(), color);
        ctr->setPalette(pal);
    };

    if (latency >= g_globals->params.orderBookParameters.asksSide.latencyLowMin <= g_globals->params.orderBookParameters.asksSide.latencyLowMax)
    {
        changeColor(m_ui->asksLatency, g_globals->colors.orderBookDockColors.asksSide.latencyLow);
    }
    else if (latency >= g_globals->params.orderBookParameters.asksSide.latencyMediumMin && latency <= g_globals->params.orderBookParameters.asksSide.latencyMediumMin)
    {
        changeColor(m_ui->asksLatency, g_globals->colors.orderBookDockColors.asksSide.latencyMedium);
    }
    else if (latency >= g_globals->params.orderBookParameters.asksSide.latencyHighMin && latency <= g_globals->params.orderBookParameters.asksSide.latencyHighMax)
    {
        changeColor(m_ui->asksLatency, g_globals->colors.orderBookDockColors.asksSide.latencyHigh);
    }

    if (latency >= g_globals->params.orderBookParameters.bidsSide.latencyLowMin <= g_globals->params.orderBookParameters.bidsSide.latencyLowMax)
    {
        changeColor(m_ui->bidsLatency, g_globals->colors.orderBookDockColors.bidsSide.latencyLow);
    }
    else if (latency >= g_globals->params.orderBookParameters.bidsSide.latencyMediumMin && latency <= g_globals->params.orderBookParameters.bidsSide.latencyMediumMin)
    {
        changeColor(m_ui->bidsLatency, g_globals->colors.orderBookDockColors.bidsSide.latencyMedium);
    }
    else if (latency >= g_globals->params.orderBookParameters.bidsSide.latencyHighMin && latency <= g_globals->params.orderBookParameters.bidsSide.latencyHighMax)
    {
        changeColor(m_ui->bidsLatency, g_globals->colors.orderBookDockColors.bidsSide.latencyHigh);
    }

    m_ui->asksLatency->setText(QString("%1 ms").arg(latency));
    m_ui->bidsLatency->setText(QString("%1 ms").arg(latency));

    if (m_ui->m_dockDepth->isVisible())
    {
        plotDepth(asks, bids);
    }
}

void CENTAUR_NAMESPACE::CentaurApp::plotDepth(const QMap<QString, QPair<QString, QString>> &asks, const QMap<QString, QPair<QString, QString>> &bids) noexcept
{
    auto generatePoints = [](const QMap<QString, QPair<QString, QString>> &data, QVector<double> &prices, QVector<double> &increments) {
        double prevQuant = 0.0;
        for (auto iter = data.begin(); iter != data.end(); ++iter)
        {
            const auto &obprices = iter.key();
            auto quantity        = iter.value().first;
            if (!quantity.isEmpty() && !quantity[0].isDigit())
            {
                quantity = quantity.remove(0, 1).mid(0);
                quantity.replace(",", "");
            }

            const auto &price = obprices.toDouble();
            const auto &quant = quantity.toDouble() + prevQuant;

            increments.push_back(quant);
            prices.push_back(price);

            prevQuant = quant;
        }
    };
    using minMaxType    = std::pair<std::tuple<double, double, double>, std::tuple<double, double, double>>;
    auto generateMinMax = [](const QVector<double> &x, const QVector<double> &y) -> minMaxType {
        double minY         = y.first(),
               maxY         = y.last() * 2;
        double stepsY       = (maxY - minY) / 10.0;

        const double minX   = x.first(),
                     maxX   = x.last();
        const double stepsX = (maxX - minX) / 5.0;

        return {
            { minX, maxX, stepsX },
            { minY, maxY, stepsY }
        };
    };

    QVector<double> bidsX, bidsY, asksX, asksY;
    generatePoints(asks, asksX, asksY);
    generatePoints(bids, bidsX, bidsY);

    auto [rangeAsksX, rangeAsksY]          = generateMinMax(asksX, asksY);
    auto [rangeBidsX, rangeBidsY]          = generateMinMax(bidsX, bidsY);

    auto &[asksMinX, asksMaxX, asksStepsX] = rangeAsksX;
    auto &[asksMinY, asksMaxY, asksStepsY] = rangeAsksY;

    auto &[bidsMinX, bidsMaxX, bidsStepsX] = rangeBidsX;
    auto &[bidsMinY, bidsMaxY, bidsStepsY] = rangeBidsY;

    double maxY                            = std::max(bidsMaxY, asksMaxY);

    //   reinterpret_cast<QwtLinearScaleEngine *>(m_ui->m_asksDepth->axisScaleEngine(QwtPlot::Axis::yRight))->autoScale(10, asksMinY, maxY, asksStepsY);
    //   m_ui->m_asksDepth->setAxisScale(QwtPlot::Axis::yRight, asksMinY, maxY, asksStepsY);
    //   m_ui->m_asksDepth->setAxisAutoScale(QwtPlot::Axis::yRight, true);

    //  m_ui->m_asksDepth->setAxisScale(QwtPlot::Axis::xBottom, asksMinX, asksMaxX, asksStepsX);
    //   m_plotAsksDepth->setSamples(asksX, asksY);

    //   reinterpret_cast<QwtLinearScaleEngine *>(m_ui->m_bidsDepth->axisScaleEngine(QwtPlot::Axis::yLeft))->autoScale(10, bidsMinY, maxY, bidsStepsY);
    // m_ui->m_bidsDepth->setAxisScale(QwtPlot::Axis::yLeft, bidsMinY, maxY, bidsStepsY);
    // m_ui->m_bidsDepth->setAxisAutoScale(QwtPlot::Axis::yLeft, true);

    // m_ui->m_bidsDepth->setAxisScale(QwtPlot::Axis::xBottom, bidsMaxX, bidsMinX, bidsStepsX);
    // m_plotBidsDepth->setSamples(bidsX, bidsY);

    // m_ui->m_bidsDepth->replot();
    // m_ui->m_asksDepth->replot();
}

void CENTAUR_NAMESPACE::CentaurApp::clearOrderbookListsAndDepth() noexcept
{
    m_currentViewOrderbookSymbol = { "", "" };
    m_ui->bidsSymbol->setText("");
    m_ui->asksSymbol->setText("");
    m_ui->bidsLatency->setText("");
    m_ui->asksLatency->setText("");
    m_ui->asksTable->setRowCount(0);
    m_ui->bidsTable->setRowCount(0);

    double *dnptr = nullptr;
    //  m_plotAsksDepth->setSamples(dnptr, 0);
    //  m_plotBidsDepth->setSamples(dnptr, 0);

    //  m_ui->m_bidsDepth->replot();
    //  m_ui->m_asksDepth->replot();
}

void CENTAUR_NAMESPACE::CentaurApp::onPlugins() noexcept
{
    CENTAUR_NAMESPACE::PluginsDialog dlg(this);
    dlg.exec();
}

void cen::CentaurApp::addFavoritesWatchListDB(const QString &symbol, const QString &sender) noexcept
{
    m_sqlFavorites->add(symbol, sender);
}

void cen::CentaurApp::removeFavoritesWatchListDB(const QString &symbol, const QString &sender) noexcept
{
    m_sqlFavorites->del(symbol, sender);
}
