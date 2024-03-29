/////////////////////////////////////////////////////////////////////////////////////
//
// Created by Ricardo Romero on 09/11/22.
// Copyright (c) 2022 Ricardo Romero.  All rights reserved.
//

#include "LogDialog.hpp"
#include "../ui/ui_LogDialog.h"
#include "CentaurInterface.hpp"
#include <QAbstractTextDocumentLayout>
#include <QDateTime>
#include <QPainter>
#include <QSettings>
#include <QStyledItemDelegate>
#include <QTextDocument>

BEGIN_CENTAUR_NAMESPACE

namespace
{
    class HTMLDelegate : public QStyledItemDelegate
    {
    public:
        HTMLDelegate() = default;

        ~HTMLDelegate() override = default;

    protected:
        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
        {
            QStyleOptionViewItem options = option;
            initStyleOption(&options, index);

            painter->save();

            QTextDocument doc;
            doc.setHtml(options.text);

            options.text = "";
            options.widget->style()->drawControl(QStyle::CE_ItemViewItem, &options, painter, option.widget);

            // shift text right to make icon visible
            const QSize iconSize = options.icon.actualSize(options.rect.size());
            painter->translate(options.rect.left() + iconSize.width(), options.rect.top());
            const QRect clip(0, 0, options.rect.width() + iconSize.width(), options.rect.height());

            painter->setClipRect(clip);
            QAbstractTextDocumentLayout::PaintContext ctx;

            ctx.clip = clip;

            doc.setTextWidth(option.rect.width());
            doc.documentLayout()->draw(painter, ctx);

            painter->restore();
        }

        C_NODISCARD inline QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
        {
            QStyleOptionViewItem options = option;
            initStyleOption(&options, index);

            QTextDocument doc;
            doc.setHtml(options.text);
            doc.setTextWidth(options.rect.width());
            return { static_cast<int>(doc.idealWidth()), static_cast<int>(doc.size().height()) };
        }
    };
} // namespace

struct LogDialog::Impl
{
    inline explicit Impl(QDialog *parent) :
        ui { new Ui::LogDialog }
    {
        ui->setupUi(parent);
    }

    inline ~Impl() = default;

    std::unique_ptr<Ui::LogDialog> ui;
};

LogDialog::LogDialog(QWidget *parent) :
    CDialog { parent },
    _impl { new Impl(this) }
{

    ui()->titleFrame->setFrameTitle(tr("Logs"));

    QTableWidget *logger = ui()->logsTable;
    logger->setHorizontalHeaderLabels({ tr("Date"), tr("User"), tr("Session"), tr("Type"), tr("Source"), tr("Message") });
    //    logger->horizontalHeaderItem(0)->setFont(QFont("Roboto", 10));
    logger->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    //    logger->horizontalHeaderItem(1)->setFont(QFont("Roboto", 10));
    logger->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    //    logger->horizontalHeaderItem(2)->setFont(QFont("Roboto", 10));
    logger->horizontalHeaderItem(2)->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    //    logger->horizontalHeaderItem(3)->setFont(QFont("Roboto", 10));
    logger->horizontalHeaderItem(3)->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    //    logger->horizontalHeaderItem(4)->setFont(QFont("Roboto", 10));
    logger->horizontalHeaderItem(4)->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    //    logger->horizontalHeaderItem(5)->setFont(QFont("Roboto", 10));
    logger->horizontalHeaderItem(5)->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    logger->setItemDelegateForColumn(5, new HTMLDelegate);

    restoreInterface();
}

LogDialog::~LogDialog() = default;

Ui::LogDialog *LogDialog::ui()
{
    return _impl->ui.get();
}

void LogDialog::restoreInterface() noexcept
{
    restoreDialogInterface();

    QSettings settings;
    settings.beginGroup("LogDialog_Table");
    settings.setValue("geometry", ui()->logsTable->saveGeometry());
    settings.setValue("h-geometry", ui()->logsTable->horizontalHeader()->saveGeometry());
    settings.setValue("state", ui()->logsTable->horizontalHeader()->saveState());
    settings.endGroup();
}

QTableWidget *LogDialog::tableWidget()
{
    return ui()->logsTable;
}

void LogDialog::onLog(qint64 date, int session, int level, const QString &usr, const QString &source, const QString &msg) noexcept
{
    QTableWidget *logger = ui()->logsTable;
    if (logger->rowCount() > 2000)
        logger->clearContents();

    auto logLevel = static_cast<CENTAUR_NAMESPACE::interface::LogLevel>(level);

    int curRow = logger->rowCount();
    logger->insertRow(curRow);

    auto insertItem = [&](const int &col, const QString &text, const QColor &color, const Qt::Alignment &align = Qt::AlignCenter) {
        auto *item = new QTableWidgetItem(text);
        // item->setFont(QFont("Roboto", 12));
        item->setForeground(color);
        item->setTextAlignment(align | Qt::AlignVCenter);
        logger->setItem(curRow, col, item);
        return item;
    };

    auto *insertedItem = insertItem(0,
        QDateTime::fromSecsSinceEpoch(static_cast<qint64>(date)).toString("dd-MM-yyyy hh:mm:ss.zzz"),
        QColor(0xA2A2A2), Qt::AlignLeft | Qt::AlignVCenter);

    insertItem(1, usr, Qt::white);
    insertItem(2, QString("%1").arg(session), Qt::white);

    switch (logLevel) {
        case CENTAUR_NAMESPACE::interface::LogLevel::fatal:
            insertItem(3, "fatal", Qt::red);
            break;
        case CENTAUR_NAMESPACE::interface::LogLevel::error:
            insertItem(3, "error", QColor(0xFF8C00ul)); // Dark orange
            break;
        case CENTAUR_NAMESPACE::interface::LogLevel::warning:
            insertItem(3, "warning", Qt::yellow); // Dark orange
            break;
        case CENTAUR_NAMESPACE::interface::LogLevel::info:
            insertItem(3, "info", QColor(0x00BFFFul)); // Deep sky blue
            break;
        case CENTAUR_NAMESPACE::interface::LogLevel::debug:
            insertItem(3, "debug", QColor(0x6A5ACDul)); // Slate blue
            break;
        case CENTAUR_NAMESPACE::interface::LogLevel::trace:
            insertItem(3, "trace", QColor(0xFF69B4ul)); // Hot pink
            break;
    }

    insertItem(4, source, Qt::white);

    QString newMsg { msg };

    newMsg.prepend("<span style=\"vertical-align: middle; line-height: 25px;\">");
    auto idx = newMsg.indexOf("#", 0);
    if (idx != -1) {
        do {
            newMsg.replace(idx, 1, "<span style=\"color:");
            idx = newMsg.indexOf("#", idx + static_cast<int>(QString("<span style=\"color:").size()) + 1);
            newMsg.replace(idx, 1, "\">");
            idx = newMsg.indexOf("#", idx + static_cast<int>(QString("\">").size()));
            newMsg.replace(idx, 1, "</span>");
        } while ((idx = newMsg.indexOf("#", idx)) != -1);
    }
    newMsg.append("</span>");

    insertItem(5, newMsg, Qt::white, Qt::AlignLeft);

    logger->scrollToItem(insertedItem);
}

void LogDialog::saveInterface() noexcept
{
    CDialog::saveInterface();

    QSettings settings;
    settings.beginGroup("LogDialog_Table");
    settings.setValue("geometry", tableWidget()->saveGeometry());
    settings.setValue("h-geometry", tableWidget()->horizontalHeader()->saveGeometry());
    settings.setValue("state", tableWidget()->horizontalHeader()->saveState());
    settings.endGroup();
}

END_CENTAUR_NAMESPACE
