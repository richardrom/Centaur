/////////////////////////////////////////////////////////////////////////////////////
//
// Created by Ricardo Romero on 09/12/21.
// Copyright (c) 2021 Ricardo Romero.  All rights reserved.
//

#include "Logger.hpp"
#include "CentaurInterface.hpp"
#include "LogDialog.hpp"
#include "QtSql/qsqlquery.h"
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QStandardPaths>
#include <QTranslator>
#include <string>

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexit-time-destructors"
#pragma clang diagnostic ignored "-Wglobal-constructors"
#endif /*__clang__*/

CENTAUR_NAMESPACE::CentaurLogger *CENTAUR_NAMESPACE::g_logger { nullptr };

#ifdef __clang__
#pragma clang diagnostic pop
#endif /*__clang__*/

namespace
{
    using namespace Qt::Literals::StringLiterals;

    auto sqlExec(void *, int, char **, char **) -> int
    {
        // The database function does not really care about selecting data
        // We only do inserts
        return 0;
    }
} // namespace

CENTAUR_NAMESPACE::CentaurLogger::CentaurLogger()
{
    sqlite3_initialize();
}

CENTAUR_NAMESPACE::CentaurLogger::~CentaurLogger()
{
    if (m_sql != nullptr)
        sqlite3_close(m_sql);

    sqlite3_shutdown();
}

void CENTAUR_NAMESPACE::CentaurLogger::run() noexcept
{
    while (!m_terminateSignal) {
        // Protect Race Conditions
        std::unique_lock<std::mutex> lock { m_dataProtect };
        // Wait for the termination flag or input data
        m_waitCondition.wait(lock, [&]() { return m_terminateSignal || !m_messages.empty(); });

        dispatch();
    }
}

void CENTAUR_NAMESPACE::CentaurLogger::terminate() noexcept
{
    m_terminateSignal = true;
    m_waitCondition.notify_one();
}

void CENTAUR_NAMESPACE::CentaurLogger::process(const LogMessage &log) noexcept
{
    QMetaObject::invokeMethod(m_app->logDialog(), "onLog",
        Qt::QueuedConnection,
        Q_ARG(qint64, log.date),
        Q_ARG(int, log.session),
        Q_ARG(int, static_cast<int>(log.level)),
        Q_ARG(QString, log.user),
        Q_ARG(QString, log.source),
        Q_ARG(QString, log.msg));

    //  Insert into database
    const auto query = QString(R"(INSERT INTO log (date, session, user, level, source, message) VALUES ('%1', %2, '%3', %4, '%5', '%6');)")
                           .arg(QDateTime::fromSecsSinceEpoch(log.date).toString("dd-MM-yyyy hh:mm:ss.zzz"))
                           .arg(log.session)
                           .arg(log.user)
                           .arg(static_cast<int>(log.level))
                           .arg(log.source, [message = log.msg]() mutable -> QString {
                               auto colorStarts = message.indexOf("##");

                               while (colorStarts >= 0) {
                                   const auto color = message.indexOf("#", colorStarts + 2);
                                   if (color == -1) {
                                       colorStarts = -1;
                                       continue;
                                   }

                                   const auto colorEnds = message.indexOf("#", color + 1);
                                   if (colorEnds == -1) {
                                       colorStarts = -1;
                                       continue;
                                   }

                                   message.erase(
                                       std::next(message.cbegin(), colorEnds),
                                       std::next(message.cbegin(), (colorEnds + 1)));
                                   message.erase(
                                       std::next(message.cbegin(), color),
                                       std::next(message.cbegin(), color + 1));
                                   message.erase(
                                       std::next(message.cbegin(), colorStarts),
                                       std::next(message.cbegin(), color));

                                   colorStarts = message.indexOf("##");
                               }

                               return message;
                           }());

    char *errStr  = nullptr;
    const int err = sqlite3_exec(m_sql, qPrintable(query), sqlExec, nullptr, &errStr);

    if (err != SQLITE_OK) {
        QMetaObject::invokeMethod(m_app->logDialog(), "onLog",
            Qt::QueuedConnection,
            Q_ARG(unsigned long long, log.date),
            Q_ARG(int, log.session),
            Q_ARG(int, static_cast<int>(log.level)),
            Q_ARG(QString, QString { "logger" }),
            Q_ARG(QString, QString { "insert" }),
            Q_ARG(QString, QString { errStr }));
        sqlite3_free(errStr);
    }
}

void CENTAUR_NAMESPACE::CentaurLogger::setUser(const QString &user) noexcept
{
    const std::unique_lock<std::mutex> lock { m_dataProtect };
    m_user = user;
}

void CENTAUR_NAMESPACE::CentaurLogger::updateSession() noexcept
{
    // Lint on this code is primarily due to the mix of the C-style code in sqlite3 interface
    // NOLINTBEGIN
    auto callback = [](void *param, int argc, char **data, char **cols) -> int {
        int *session = reinterpret_cast<int *>(param);
        if (argc == 1 && strcmp(cols[0], "mx") == 0) {
            *session = std::stoi(data[0]) + 1;
        }

        return 0;
    };

    char *errStr  = nullptr;
    const int err = sqlite3_exec(m_sql, R"(SELECT max(session) AS mx FROM log;)", callback, reinterpret_cast<void *>(&m_session), &errStr);

    if (err != SQLITE_OK) {
        log("internal", interface::LogLevel::error, "Session id was not retrieved");
        sqlite3_free(errStr);
    }
    // NOLINTEND
}

void CENTAUR_NAMESPACE::CentaurLogger::dispatch() noexcept
{
    // Process all queries
    while (!m_messages.empty()) {
        // Get Data
        const LogMessage msg = m_messages.front();
        // Process Data
        process(msg);
        // Remove it from the queries queue
        m_messages.pop();
    }
}

void CENTAUR_NAMESPACE::CentaurLogger::setApplication(CENTAUR_NAMESPACE::CentaurApp *app)
{
    const QString appDataLocation = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    auto logFile                  = appDataLocation + "/Log/log0.db";
    m_app                         = app;

    bool recoverDb = false;

    if (!QFileInfo::exists(logFile)) {
        const int iResult = QMessageBox::warning(app,
            QApplication::tr("Error"),
            QApplication::tr("Could not locate logging file\nWould you like to try to recover?"),
            QMessageBox::Yes | QMessageBox::No);

        if (iResult == QMessageBox::Yes) {
            recoverDb = true;
        }
        else
            throw std::runtime_error("file not located");
    }

    QString recoverQuery;
    if (recoverDb) {
        const QFileInfo fileInformation(logFile);
        auto logFileInfoDir = fileInformation.dir();

        logFileInfoDir.mkpath(logFileInfoDir.absolutePath());

        auto recoveryFile = g_globals->paths.appPath + "/Contents/Resources/Repair/logdb.sql";
        QFile file(recoveryFile);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            throw std::runtime_error("recovery file not located");

        QTextStream text(&file);
        recoverQuery = text.readAll();
    }

    if (const int err = sqlite3_open(logFile.toStdString().c_str(), &m_sql); err != SQLITE_OK)
        throw(std::runtime_error(sqlite3_errstr(err)));

    if (recoverDb) {
        char *errStr  = nullptr;
        const int err = sqlite3_exec(m_sql, recoverQuery.toStdString().c_str(), sqlExec, nullptr, &errStr);

        if (err != SQLITE_OK) {
            const std::string str { errStr };
            sqlite3_free(errStr);
            throw std::runtime_error(str);
        }
    }

    updateSession();
}

void CENTAUR_NAMESPACE::CentaurLogger::log(const QString &source, CENTAUR_NAMESPACE::interface::LogLevel level, const QString &msg) noexcept
{
    const std::lock_guard<std::mutex> lock { m_dataProtect };
    m_messages.push({ std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count(),
        m_session,
        level,
        m_user,
        source,
        msg });
    m_waitCondition.notify_one();
}
