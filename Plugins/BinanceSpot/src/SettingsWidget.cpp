/////////////////////////////////////////////////////////////////////////////////////
//
// Created by Ricardo Romero on 20/05/23.
// Copyright (c) 2023 Ricardo Romero.  All rights reserved.
//

#include "SettingsWidget.hpp"
#include "../ui/ui_SettingsWidget.h"
#include "BinanceSPOT.hpp"
#include "LinkedLineEdit.hpp"
#include "QtCore/qnamespace.h"
#include "QtWidgets/qcheckbox.h"
#include "QtWidgets/qmessagebox.h"
#include <QMessageBox>
#include <QSettings>

BEGIN_CENTAUR_NAMESPACE

struct SettingsWidget::Impl
{
    inline explicit Impl(BinanceSpotPlugin *plugin, CENTAUR_INTERFACE_NAMESPACE::IConfiguration *config) :
        ui { new Ui::SettingsWidget },
        binanceSpotPlugin { plugin },
        configuration { config }
    {
    }
    inline ~Impl() = default;

    std::unique_ptr<Ui::SettingsWidget> ui;

    BinanceSpotPlugin *binanceSpotPlugin;
    CENTAUR_INTERFACE_NAMESPACE::IConfiguration *configuration;

    bool initialUserDataState { false };
    bool modifiedCredentials { false };
};

SettingsWidget::SettingsWidget(CENTAUR_PLUGIN_NAMESPACE::IBase *plugin, CENTAUR_INTERFACE_NAMESPACE::IConfiguration *config, QWidget *parent) :
    QWidget(parent),
    _impl(new Impl { dynamic_cast<BinanceSpotPlugin *>(plugin), config })
{
    ui()->setupUi(this);

#ifdef Q_OS_MAC
    setWindowFlag(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
#endif

    ui()->secretKey->linkLabel(ui()->secretLabel);
    ui()->apiKey->linkLabel(ui()->apiLabel);

    auto enableInput = [this]() {
        ui()->secretLabel->setEnabled(true);
        ui()->secretKey->setEnabled(true);
        ui()->apiLabel->setEnabled(true);
        ui()->apiKey->setEnabled(true);
    };

    auto disableInput = [this]() {
        ui()->secretLabel->setEnabled(false);
        ui()->secretKey->setEnabled(false);
        ui()->apiLabel->setEnabled(false);
        ui()->apiKey->setEnabled(false);
    };

    connect(ui()->allowUserData, &QCheckBox::stateChanged, this,
        [this, enableInput, disableInput](int state) {
            if (state == Qt::CheckState::Checked)
            {
                enableInput();
                _impl->binanceSpotPlugin->pluginSettings["allowUserData"].SetBool(true);
            }
            else
            {
                disableInput();
                _impl->binanceSpotPlugin->pluginSettings["allowUserData"].SetBool(false);
            }
        });

    if (_impl->binanceSpotPlugin->pluginSettings["allowUserData"].GetBool())
    {
        _impl->initialUserDataState = true;

        ui()->apiKey->setText(_impl->binanceSpotPlugin->pluginSettings["user"]["api"].GetString());
        ui()->secretKey->setText(".......................................................");

        ui()->allowUserData->setCheckState(Qt::CheckState::Checked);
        enableInput();
    }
    else
    {
        ui()->allowUserData->setCheckState(Qt::CheckState::Unchecked);
        disableInput();
    }

    connect(ui()->apiKey, &LinkedLineEdit::editingFinished, this,
        [&]() {
            _impl->modifiedCredentials = true;
        });
    connect(ui()->secretKey, &LinkedLineEdit::editingFinished, this, [&]() { _impl->modifiedCredentials = true; });
}

SettingsWidget::~SettingsWidget()
{
    const bool confUserState = _impl->binanceSpotPlugin->pluginSettings["allowUserData"].GetBool();

    auto &userDataJObject = _impl->binanceSpotPlugin->pluginSettings;

    if (!userDataJObject.HasMember("user"))
    {
        userDataJObject.AddMember("user", rapidjson::kObjectType, userDataJObject.GetAllocator());

        userDataJObject["user"].AddMember("api", "", userDataJObject.GetAllocator());
        userDataJObject["user"].AddMember("secret", "", userDataJObject.GetAllocator());
    }

    if ((_impl->initialUserDataState != confUserState && confUserState) || _impl->modifiedCredentials)
    {
        const auto apiData = ui()->apiKey->text().toUtf8();
        userDataJObject["user"]["api"].SetString(apiData.constData(), static_cast<unsigned int>(apiData.size()), userDataJObject.GetAllocator());

        try
        {
            auto cipher = _impl->configuration->credentials(
                ui()->secretKey->text().toStdString(),
                true);

            if (!cipher.empty())
            {
                userDataJObject["user"]["secret"].SetString(cipher.c_str(), static_cast<unsigned int>(cipher.size()), userDataJObject.GetAllocator());
            }
            else
            {
                QMessageBox::critical(
                    this,
                    "BinanceSPOT Settings",
                    tr("Could not acquire user credentials"),
                    QMessageBox::Ok);
            }

            _impl->binanceSpotPlugin->updateKeys(ui()->apiKey->text(), ui()->secretKey->text());

        } catch (const std::runtime_error &ex)
        {
            QMessageBox::critical(
                this,
                "BinanceSPOT Settings",
                tr("Could not store the data\n%1").arg(ex.what()),
                QMessageBox::Ok);
        }
    }

    _impl->binanceSpotPlugin->storeData();
}

Ui::SettingsWidget *SettingsWidget::ui()
{
    return _impl->ui.get();
}

END_CENTAUR_NAMESPACE
