/////////////////////////////////////////////////////////////////////////////////////
//
// Created by Ricardo Romero on 27/02/23.
// Copyright (c) 2023 Ricardo Romero.  All rights reserved.
//

// You may need to build the project (run Qt uic code generator) to get "ui_SettingsWidget.h" resolved

#include "SettingsWidget.hpp"
#include "../ui/ui_SettingsWidget.h"
#include "QtCore/qnamespace.h"

#include <QComboBox>
#include <QMessageBox>
#include <QSettings>
#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>

#include <fstream>

namespace
{
    class ComboBoxDelegate : public QStyledItemDelegate
    {
    public:
        explicit ComboBoxDelegate(QTableView *parentTable, cen::plugin::ExchangeRatePlugin *exchRatePlg, QObject *parent = nullptr) :
            QStyledItemDelegate { parent },
            _plugin { exchRatePlg },
            _parentTable { parentTable }
        {
        }

        ~ComboBoxDelegate() override = default;

        QWidget *createEditor(QWidget *parent,
            C_UNUSED const QStyleOptionViewItem &option,
            C_UNUSED const QModelIndex &index) const override
        {
            return new QComboBox(parent);
        }

        void setEditorData(QWidget *editor, const QModelIndex &index) const override
        {
            if (!_plugin->pluginSettings.HasMember("user-enabled"))
                return;

            QStringList supportedPairs;
            for (const auto &available : _plugin->pluginSettings["user-enabled"].GetArray())
            {
                supportedPairs.push_back(available.GetString());
            }

            qobject_cast<QComboBox *>(editor)->addItems(supportedPairs);
            int idx = qobject_cast<QComboBox *>(editor)->findText(index.data().toString());
            if (idx != -1)
                qobject_cast<QComboBox *>(editor)->setCurrentIndex(idx);
        }

        void destroyEditor(C_UNUSED QWidget *editor, const QModelIndex &index) const override
        {
            if (index.data(Qt::UserRole + 1).toInt() == 1)
            {
                const QString data = index.data().toString();

                if (data.isEmpty())
                {
                    QMessageBox::warning(_parentTable, tr("Empty field"), tr("Data can not be empty. The row will be deleted"), QMessageBox::Ok);
                    _parentTable->model()->removeRow(index.row());
                    return;
                }
                else
                {
                    if (index.column() == 0)
                    {
                        _parentTable->model()->setData(index, 0, Qt::UserRole + 1);
                        _parentTable->edit(_parentTable->model()->index(index.row(), 1));
                    }
                }
            }
        }

        void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override
        {
            const QString currentData = index.data().toString();

            auto *widgetModel = qobject_cast<QSortFilterProxyModel *>(model);
            auto *comboBox    = qobject_cast<QComboBox *>(editor);

            if (currentData == comboBox->currentText())
                return;

            auto nonEditingIndex = widgetModel->index(index.row(), index.column() == 0 ? 1 : 0);

            const QString oldBase  = index.column() == 0 ? index.data().toString() : nonEditingIndex.data().toString();
            const QString oldQuote = index.column() == 1 ? index.data().toString() : nonEditingIndex.data().toString();

            const QString oldSymbol = oldBase + oldQuote;

            const QString newBase  = index.column() == 0 ? comboBox->currentText() : nonEditingIndex.data().toString();
            const QString newQuote = index.column() == 1 ? comboBox->currentText() : nonEditingIndex.data().toString();

            const QString newSymbol = newBase + newQuote;

            bool fromNewRow = false;
            if (index.data(Qt::UserRole + 1).toInt() == 1)
            {
                fromNewRow = true;
                if (index.column() == 0)
                {
                    model->setData(index, comboBox->currentText(), Qt::DisplayRole);
                    return;
                }
                else
                    model->setData(index, 0, Qt::UserRole + 1);
            }

            auto availableObject = _plugin->pluginSettings.FindMember("available");

            if (availableObject != _plugin->pluginSettings.MemberEnd())
            {
                auto existingSymbol = availableObject->value.FindMember(newSymbol.toUtf8().data());
                if (existingSymbol != availableObject->value.MemberEnd())
                {
                    QMessageBox::warning(editor, tr("Warning"), tr("There is already an existing symbol with the same quote and base"), QMessageBox::Ok);
                    if (fromNewRow)
                    {
                        _parentTable->model()->removeRow(index.row());
                    }
                    return;
                }

                auto deletingSymbol = availableObject->value.FindMember(oldSymbol.toUtf8().data());
                if (deletingSymbol != availableObject->value.MemberEnd())
                {
                    // Remove the old data
                    availableObject->value.RemoveMember(oldSymbol.toUtf8().data());
                }

                rapidjson::Value symbolItem(rapidjson::kObjectType);

                rapidjson::Value baseItem;
                baseItem.SetString(newBase.toUtf8().constData(), static_cast<unsigned int>(newBase.toUtf8().size()), _plugin->pluginSettings.GetAllocator());
                rapidjson::Value quoteItem;
                quoteItem.SetString(newQuote.toUtf8().constData(), static_cast<unsigned int>(newQuote.toUtf8().size()), _plugin->pluginSettings.GetAllocator());

                symbolItem.AddMember("b", baseItem, _plugin->pluginSettings.GetAllocator());
                symbolItem.AddMember("q", quoteItem, _plugin->pluginSettings.GetAllocator());

                rapidjson::Value namedItem;
                namedItem.SetString(newSymbol.toUtf8().constData(), static_cast<unsigned int>(newSymbol.toUtf8().size()), _plugin->pluginSettings.GetAllocator());
                availableObject->value.AddMember(namedItem, symbolItem, _plugin->pluginSettings.GetAllocator());

                _plugin->storeData();

                model->setData(index, comboBox->currentText(), Qt::DisplayRole);
            }
        }

        void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, C_UNUSED const QModelIndex &index) const override
        {
            editor->setGeometry(option.rect);
        }

        C_NODISCARD QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
        {
            return qApp->style()->sizeFromContents(
                QStyle::CT_ComboBox,
                &option,
                QSize(option.fontMetrics.horizontalAdvance(displayText(index.data(), option.locale)), option.fontMetrics.height()));
        }

        cen::plugin::ExchangeRatePlugin *_plugin;
        QTableView *_parentTable;
    };
} // namespace

cen::plugin::SettingsWidget::SettingsWidget(IBase *iBase, CENTAUR_INTERFACE_NAMESPACE::IConfiguration *config, QWidget *parent) :
    QWidget(parent),
    ui { new Ui::SettingsWidget },
    m_exchRatePlg { dynamic_cast<ExchangeRatePlugin *>(iBase) },
    m_config { config }
{
    ui->setupUi(this);

    m_symsItemModel = new QStandardItemModel(0, 2, ui->supportedSymTable);
    auto proxyModel = new QSortFilterProxyModel(this);

    proxyModel->setFilterKeyColumn(-1);
    proxyModel->setSourceModel(m_symsItemModel);
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxyModel->setRecursiveFilteringEnabled(true);

    ui->supportedSymTable->setModel(proxyModel);

    connect(ui->searchSymbols, &QLineEdit::textChanged, proxyModel, &QSortFilterProxyModel::setFilterFixedString);

    m_symsItemModel->setHorizontalHeaderLabels({ tr("Symbol"), tr("Description") });

    QSettings settings("CentaurProject", "Centaur");
    settings.beginGroup("ExchRate_SymTable");
    ui->supportedSymTable->restoreGeometry(settings.value("geometry").toByteArray());
    ui->supportedSymTable->horizontalHeader()->restoreGeometry(settings.value("h-geometry").toByteArray());
    ui->supportedSymTable->horizontalHeader()->restoreState(settings.value("state").toByteArray());
    settings.endGroup();

    bool userEnabledObjectExists = true;
    if (m_exchRatePlg->pluginSettings.FindMember("user-enabled") == m_exchRatePlg->pluginSettings.MemberEnd())
    {
        userEnabledObjectExists                       = false;
        rapidjson::Document::AllocatorType &allocator = m_exchRatePlg->pluginSettings.GetAllocator();
        rapidjson::Value userEnabledObject(rapidjson::kArrayType);
        m_exchRatePlg->pluginSettings.AddMember("user-enabled", userEnabledObject, allocator);
        m_exchRatePlg->storeData();
    }

    if (m_exchRatePlg->pluginSettings.HasMember("cache"))
    {
        for (const auto &ch : m_exchRatePlg->pluginSettings["cache"].GetArray())
        {
            const QString cur { ch["currency"].GetString() };
            const QString desc { ch["description"].GetString() };

            auto itemCur  = new QStandardItem(cur);
            auto itemDesc = new QStandardItem(desc);

            itemCur->setCheckable(true);

            if (userEnabledObjectExists)
            {
                auto enabledIter = std::find(m_exchRatePlg->pluginSettings["user-enabled"].GetArray().begin(), m_exchRatePlg->pluginSettings["user-enabled"].GetArray().end(), cur.toStdString().c_str());

                if (enabledIter != m_exchRatePlg->pluginSettings["user-enabled"].GetArray().end())
                {
                    itemCur->setCheckState(Qt::CheckState::Checked);
                }
            }

            m_symsItemModel->insertRow(m_symsItemModel->rowCount(), { itemCur, itemDesc });
        }
    }

    connect(m_symsItemModel, &QStandardItemModel::itemChanged, this, [&](QStandardItem *item) {
        auto itemCheckState = item->checkState();

        rapidjson::Document::AllocatorType &allocator = m_exchRatePlg->pluginSettings.GetAllocator();

        const std::string text = item->text().toStdString();

        switch (itemCheckState)
        {
            case Qt::Unchecked:
                [[likely]];
                {
                    auto enabledIter = std::find(
                        m_exchRatePlg->pluginSettings["user-enabled"].GetArray().begin(),
                        m_exchRatePlg->pluginSettings["user-enabled"].GetArray().end(),
                        text.c_str());

                    if (enabledIter != m_exchRatePlg->pluginSettings["user-enabled"].GetArray().end())
                    {
                        m_exchRatePlg->pluginSettings["user-enabled"].Erase(enabledIter);
                        m_exchRatePlg->storeData();
                    }
                }
                break;

            case Qt::Checked:
                [[likely]];
                {
                    auto enabledIter = std::find(
                        m_exchRatePlg->pluginSettings["user-enabled"].GetArray().begin(),
                        m_exchRatePlg->pluginSettings["user-enabled"].GetArray().end(),
                        text.c_str());

                    if (enabledIter == m_exchRatePlg->pluginSettings["user-enabled"].GetArray().end())
                    {
                        rapidjson::Value textValue(text.c_str(), allocator);
                        m_exchRatePlg->pluginSettings["user-enabled"].PushBack(textValue, allocator);
                        m_exchRatePlg->storeData();
                    }
                }
                break;

            case Qt::PartiallyChecked:
                [[unlikely]];
                break;
        }
    });

    m_usedItemModel      = new QStandardItemModel(0, 2, ui->usedPairsTable);
    auto pairsProxyModel = new QSortFilterProxyModel(this);

    pairsProxyModel->setFilterKeyColumn(-1);
    pairsProxyModel->setSourceModel(m_usedItemModel);
    pairsProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    pairsProxyModel->setRecursiveFilteringEnabled(true);

    ui->usedPairsTable->setModel(pairsProxyModel);

    connect(ui->searchUsedPairs, &QLineEdit::textChanged, pairsProxyModel, &QSortFilterProxyModel::setFilterFixedString);

    m_usedItemModel->setHorizontalHeaderLabels({ tr("Base"), tr("Quote") });

    ui->usedPairsTable->setItemDelegate(new ComboBoxDelegate(ui->usedPairsTable, m_exchRatePlg));

    settings.beginGroup("ExchRate_UsedSyms");
    ui->usedPairsTable->restoreGeometry(settings.value("geometry").toByteArray());
    ui->usedPairsTable->horizontalHeader()->restoreGeometry(settings.value("h-geometry").toByteArray());
    ui->usedPairsTable->horizontalHeader()->restoreState(settings.value("state").toByteArray());
    settings.endGroup();

    QString defQuote, defBase;
    if (m_exchRatePlg->pluginSettings.HasMember("default")
        && m_exchRatePlg->pluginSettings["default"].HasMember("quote")
        && m_exchRatePlg->pluginSettings["default"].HasMember("base"))
    {
        defQuote = m_exchRatePlg->pluginSettings["default"]["quote"].GetString();
        defBase  = m_exchRatePlg->pluginSettings["default"]["base"].GetString();
    }

    for (auto available = m_exchRatePlg->pluginSettings["available"].MemberBegin(); available != m_exchRatePlg->pluginSettings["available"].MemberEnd(); ++available)
    {
        if (available->value.HasMember("q") && available->value.HasMember("b"))
        {
            const QString itemBaseString  = available->value["b"].GetString();
            const QString itemQuoteString = available->value["q"].GetString();

            auto itemBase  = new QStandardItem(itemBaseString);
            auto itemQuote = new QStandardItem(itemQuoteString);

            itemBase->setCheckable(true);

            if (itemBaseString == defBase && itemQuoteString == defQuote)
            {
                itemBase->setCheckState(Qt::CheckState::Checked);
                m_defCheckedItem = itemBase;
            }

            m_usedItemModel->insertRow(m_usedItemModel->rowCount(), { itemBase, itemQuote });
        }
    }

    connect(ui->addPairButton, &QPushButton::released, this, [&]() {
        ui->searchUsedPairs->clear();

        auto nextRow = m_usedItemModel->rowCount();

        auto itemBase  = new QStandardItem("");
        auto itemQuote = new QStandardItem("");

        itemBase->setCheckable(true);

        itemBase->setData(1);
        itemQuote->setData(1);

        m_usedItemModel->insertRow(nextRow, { itemBase, itemQuote });

        auto *model = qobject_cast<QSortFilterProxyModel *>(ui->usedPairsTable->model());

        auto editIndex = model->mapFromSource(m_usedItemModel->indexFromItem(itemBase));

        ui->usedPairsTable->setCurrentIndex(editIndex);
        ui->usedPairsTable->edit(editIndex);
    });

    connect(ui->usedPairsTable, &DeletableTable::deleteKeyPressed, this, [&]() {
        const auto response = QMessageBox::question(
            this,
            tr("Delete item"),
            tr("Delete this item?"),
            QMessageBox::Yes | QMessageBox::No);

        if (response == QMessageBox::No)
            return;

        const auto index = ui->usedPairsTable->currentIndex();
        if (!index.isValid())
            return;

        auto *model = qobject_cast<QSortFilterProxyModel *>(ui->usedPairsTable->model());

        auto nonEditingIndex = model->index(index.row(), index.column() == 0 ? 1 : 0);

        const QString base  = index.column() == 0 ? index.data().toString() : nonEditingIndex.data().toString();
        const QString quote = index.column() == 1 ? index.data().toString() : nonEditingIndex.data().toString();

        const QString symbol = base + quote;

        model->removeRow(index.row());

        auto availableObject = m_exchRatePlg->pluginSettings.FindMember("available");
        if (availableObject != m_exchRatePlg->pluginSettings.MemberEnd())
        {
            auto deletingSymbol = availableObject->value.FindMember(symbol.toUtf8().data());
            if (deletingSymbol != availableObject->value.MemberEnd())
            {
                // Remove the old data
                availableObject->value.RemoveMember(symbol.toUtf8().data());
                m_exchRatePlg->storeData();
            }
        }
    });

    connect(ui->resetCacheButton, &QPushButton::released, this, [&]() {
        m_exchRatePlg->acquireFromInternet();
        m_exchRatePlg->storeData();
    });

    ui->timeframeEdit->setValidator(new QDoubleValidator(0.0, 86'400'000.0, 3, this));

    auto reloadTimerObject = m_exchRatePlg->pluginSettings.FindMember("reload-timer");
    if (reloadTimerObject != m_exchRatePlg->pluginSettings.MemberEnd())
    {
        const auto value = m_exchRatePlg->pluginSettings["reload-timer"].GetDouble();

        if (value < 1000)
        {
            ui->timeframeFrames->setCurrentIndex(0);
            ui->timeframeEdit->setText(QString("%1").arg(value, 0, 'f', 3));
        }
        else if (value >= 1000 && value < 60'000)
        {
            ui->timeframeFrames->setCurrentIndex(1);
            ui->timeframeEdit->setText(QString("%1").arg(value / 1000.0, 0, 'f', 3));
        }
        else if (value >= 60'000 && value < 3'600'000)
        {
            ui->timeframeFrames->setCurrentIndex(2);
            ui->timeframeEdit->setText(QString("%1").arg(value / 60'000.0, 0, 'f', 3));
        }
        else if (value >= 3'600'000)
        {
            ui->timeframeFrames->setCurrentIndex(3);
            ui->timeframeEdit->setText(QString("%1").arg(value / 3'600'000, 0, 'f', 3));
        }
    }

    connect(m_usedItemModel, &QStandardItemModel::itemChanged, this, [&](QStandardItem *item) {
        auto quoteIndex = m_usedItemModel->index(item->row(), 1);

        if (m_defCheckedItem != nullptr)
        {
            m_defCheckedItem->setCheckState(Qt::CheckState::Unchecked);
        }

        if (item->checkState() == Qt::CheckState::Unchecked && m_defCheckedItem == item)
        {
            m_defCheckedItem = nullptr;
        }

        if (item->checkState() == Qt::CheckState::Checked)
        {
            const auto base  = item->text();
            const auto quote = quoteIndex.data().toString();

            auto defaultObject = m_exchRatePlg->pluginSettings.FindMember("default");
            if (defaultObject != m_exchRatePlg->pluginSettings.MemberEnd())
            {
                auto baseDefault = defaultObject->value.FindMember("base");
                if (baseDefault != defaultObject->value.MemberEnd())
                {
                    baseDefault->value.SetString(base.toUtf8().constData(), static_cast<rapidjson::SizeType>(base.toUtf8().size()), m_exchRatePlg->pluginSettings.GetAllocator());
                }

                auto quoteDefault = defaultObject->value.FindMember("quote");
                if (quoteDefault != defaultObject->value.MemberEnd())
                {
                    quoteDefault->value.SetString(quote.toUtf8().constData(), static_cast<rapidjson::SizeType>(quote.toUtf8().size()), m_exchRatePlg->pluginSettings.GetAllocator());
                }

                if (auto iter = m_exchRatePlg->pluginSettings.FindMember("value-timestamp"); iter != m_exchRatePlg->pluginSettings.MemberEnd())
                {
                    // This will invalidate the data cache
                    iter->value.SetInt64(0);
                }

                m_exchRatePlg->onReloadData(true);

                m_exchRatePlg->storeData();
            }

            // Update the default item
            m_defCheckedItem = item;
        }
    });

    connect(ui->timeframeEdit, &QLineEdit::textChanged, this, [&](const QString &timeframe) {
        reloadTimer(timeframe);
    });

    connect(ui->timeframeFrames, &QComboBox::currentIndexChanged, this, [&](C_UNUSED int index) {
        reloadTimer(index);
    });
}

cen::plugin::SettingsWidget::~SettingsWidget()
{
    QSettings settings("CentaurProject", "Centaur");

    settings.beginGroup("ExchRate_SymTable");
    settings.setValue("geometry", ui->supportedSymTable->saveGeometry());
    settings.setValue("h-geometry", ui->supportedSymTable->horizontalHeader()->saveGeometry());
    settings.setValue("state", ui->supportedSymTable->horizontalHeader()->saveState());
    settings.endGroup();

    settings.beginGroup("ExchRate_UsedSyms");
    settings.setValue("geometry", ui->usedPairsTable->saveGeometry());
    settings.setValue("h-geometry", ui->usedPairsTable->horizontalHeader()->saveGeometry());
    settings.setValue("state", ui->usedPairsTable->horizontalHeader()->saveState());
    settings.endGroup();
}

void cen::plugin::SettingsWidget::reloadTimer(const QString &timeframe)
{
    const auto timeframeValue = timeframe.toDouble() * [&]() -> double {
        switch (ui->timeframeFrames->currentIndex())
        {
            case 0:
                return 1.0;
            case 1:
                return 1000.0;
            case 2:
                return 60'000.0;
            case 3:
                return 3'600'000;
            default:
                return 1.0;
        }
    }();

    reloadTimer(timeframeValue);
}

void cen::plugin::SettingsWidget::reloadTimer(int index)
{
    const auto timeframeValue = ui->timeframeEdit->text().toDouble() * [&]() -> double {
        switch (index)
        {
            case 0:
                return 1.0;
            case 1:
                return 1000.0;
            case 2:
                return 60'000.0;
            case 3:
                return 3'600'000;
            default:
                return 1.0;
        }
    }();

    reloadTimer(timeframeValue);
}

void cen::plugin::SettingsWidget::reloadTimer(double timeframe)
{
    auto reloadTimerObject = m_exchRatePlg->pluginSettings.FindMember("reload-timer");
    if (reloadTimerObject != m_exchRatePlg->pluginSettings.MemberEnd())
    {
        reloadTimerObject->value.SetDouble(timeframe);
        m_exchRatePlg->restartReloadTimer(static_cast<int>(timeframe));
        m_exchRatePlg->storeData();
    }
}
