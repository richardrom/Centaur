////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Created by Ricardo Romero on 15/04/22.
//  Copyright (c) 2022 Ricardo Romero.  All rights reserved.
//

#include "LoginDialog.hpp"
#include "../ui/ui_LoginDialog.h"
#include "CentaurApp.hpp"
#include "Globals.hpp"
#include "TOTP.hpp"
#include <Protocol.hpp>
#include <QCryptographicHash>
#include <QFile>
#include <QMessageBox>
#include <QSettings>
#include <QStandardPaths>
#include <QTextStream>
#include <QUuid>

BEGIN_CENTAUR_NAMESPACE

struct LoginDialog::Impl
{
    inline explicit Impl(QDialog *parent) :
        ui { new Ui::LoginDialog }
    {
        ui->setupUi(parent);
    }

    inline ~Impl() = default;

    bool loginTFA { false };
    bool pswMode { false };
    bool tfaMode { false };

    std::unique_ptr<Ui::LoginDialog> ui;
};

LoginDialog::LoginDialog(QWidget *parent) :
    CDialog { parent },
    _impl { new Impl(this) }
{
    setAccept(ui()->acceptButton);
}

LoginDialog::~LoginDialog() = default;

Ui::LoginDialog *LoginDialog::ui()
{
    return _impl->ui.get();
}

void LoginDialog::onAccept() noexcept
{
    QSettings settings;
    saveInterface();

    settings.beginGroup("user.password");
    auto pad = settings.value("pad").toString();
    settings.endGroup();

    settings.beginGroup("__Session__data");
    const QString registeredPassword = settings.value("__sec__").toString();
    const QString registeredUser     = settings.value("__user__").toString();
#ifndef TEST_LOGIN_MODE
    const bool loginTFA = settings.value("__2fa__").toBool();
#endif /*TEST_LOGIN_MODE*/
    settings.endGroup();

    using namespace CENTAUR_PROTOCOL_NAMESPACE;
    settings.beginGroup("__iv__");
    const auto localIV = settings.value("__local__").toString();
    settings.endGroup();

    const QString hashedPassword = QString::fromUtf8(QCryptographicHash::hash(QByteArrayView(ui()->passwordEdit->text().toUtf8()), QCryptographicHash::RealSha3_512).toBase64());

    if (!_impl->tfaMode && _impl->pswMode) {
        ui()->passwordEdit->setFocus();

        if (registeredPassword != hashedPassword) {
            ui()->errLabel->setText(tr("Wrong password"));
            ui()->errLabel->show();
            return;
        }

        if (_impl->pswMode) {
            userPassword = ui()->passwordEdit->text();
        }

        accept();
    }

    if (!_impl->pswMode && !_impl->tfaMode) {

        // Normal mode
#ifndef TEST_LOGIN_MODE // Avoid doing all the testing
        if (ui()->userEdit->text() != registeredUser) {
            ui()->errLabel->setText(tr("Wrong user"));
            ui()->errLabel->show();
            return;
        }

        if (registeredPassword != hashedPassword) {
            ui()->errLabel->setText(tr("Wrong password"));
            ui()->errLabel->show();
            return;
        }

        if (loginTFA) {
            const QString tfaFile = []() -> QString {
                QString const data = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
                return QString("%1/f6110cb58f3b").arg(data);
            }();

            QFile file(tfaFile);
            if (!file.open(QIODeviceBase::ReadOnly)) {
                // Code: 0x00000001 is encryption failed
                // Code: 0x00000002 is a file failure
                QMessageBox::critical(this, tr("Error"), QString(tr("There is an error in the user identification. 2FA is not available.")));
                exit(EXIT_FAILURE);
            }

            QTextStream stream(&file);
            QString fileData = stream.readAll();
            file.close();

            try {
                g_globals->session.userTFA = QString::fromStdString(
                    Encryption::DecryptAES(
                        fileData.toStdString(),
                        pad.replace(0, ui()->passwordEdit->text().size(), ui()->passwordEdit->text()).toStdString(),
                        localIV.toStdString()));
            } catch (const std::exception &ex) {
                return;
            }

            if (ui()->tfaEdit->text().toInt() != getTOTPCode(g_globals->session.userTFA.toStdString())) {
                ui()->errLabel->setText(tr("Wrong 2FA input"));
                ui()->errLabel->show();
                return;
            }
        }
#endif /*TEST_LOGIN_MODE*/

        settings.beginGroup("__Session__data");
        g_globals->session.user    = settings.value("__user__").toString();
        g_globals->session.display = settings.value("__display__").toString();
        g_globals->session.email   = settings.value("__email__").toString();
        g_globals->session.tfa     = settings.value("__2fa__").toBool();
        settings.endGroup();
    }

    if (_impl->tfaMode) {
        ui()->tfaEdit->setFocus();

        if (g_globals->session.userTFA.isEmpty()) {
            if (registeredPassword != registeredUser) {
                ui()->errLabel->setText(tr("Wrong password"));
                ui()->errLabel->show();
                return;
            }

            if (_impl->pswMode) {
                userPassword = ui()->passwordEdit->text();
            }

            const QString tfaFile = []() -> QString {
                QString const data = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
                return QString("%1/f6110cb58f3b").arg(data);
            }();

            QFile file(tfaFile);
            if (!file.open(QIODeviceBase::ReadOnly)) {
                // Code: 0x00000001 is encryption failed
                // Code: 0x00000002 is a file failure
                QMessageBox::critical(this, tr("Error"), QString(tr("There is an error in the user identification. 2FA is not available.")));
                exit(EXIT_FAILURE);
            }

            QTextStream stream(&file);
            const QString fileData = stream.readAll();
            file.close();

            g_globals->session.userTFA = QString::fromStdString(
                Encryption::DecryptAES(fileData.toStdString(),
                    pad.replace(0, ui()->passwordEdit->text().size(), ui()->passwordEdit->text()).toStdString(),
                    localIV.toStdString()));
        }

        if (ui()->tfaEdit->text().toInt() != getTOTPCode(g_globals->session.userTFA.toStdString())) {
            ui()->errLabel->setText(tr("Wrong 2FA input"));
            ui()->errLabel->show();
            return;
        }

        accept();
    }

    accept();
}

void LoginDialog::accept()
{
    auto *mainWindow = []() -> CentaurApp * {
        foreach (QWidget *w, QApplication::topLevelWidgets())
            if (auto *mainWin = qobject_cast<CentaurApp *>(w))
                return mainWin;
        return nullptr;
    }();

    if (g_credentials != nullptr) {
        delete g_credentials;
    }

#ifndef TEST_LOGIN_MODE
    // The 'new' approach tries to randomize addresses as much as possible
    g_credentials = new QString(this->ui()->passwordEdit->text());
#else

    /// TODO: DOCUMENT HOW THIS WORKS

    const QString fileName = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/h983mjuklg43";
    QFile file(fileName);
    if (!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this,
            tr("Error"),
            tr("When the login test mode is on. You must set the password in a file.\nCheck documentation to see how this is done"),
            QMessageBox::Ok);
        abort();
    }

    QTextStream stream(&file);
    const QString fileData   = stream.readAll();
    const QString hashedData = QString::fromUtf8(QCryptographicHash::hash(QByteArrayView(fileData.toUtf8()), QCryptographicHash::RealSha3_512).toBase64());

    QSettings settings;
    settings.beginGroup("__Session__data");
    const QString registeredPassword = settings.value("__sec__").toString();
    settings.endGroup();

    if (registeredPassword != hashedData) {
        QMessageBox::critical(this,
            tr("Error"),
            tr("The login test mode is on, and the password written is not correct.\nCheck documentation to see how this is done"),
            QMessageBox::Ok);
        abort();
    }
    file.close();

    // Set the credentials
    g_credentials = new QString(fileData);
#endif

    if (mainWindow != nullptr)
        mainWindow->keepAliveCredentialStatus();

#ifndef TEST_LOGIN_MODE // avoid closing because it's not going to work
    QDialog::accept();
#else
    if (_impl->pswMode || _impl->tfaMode)
        QDialog::accept();
#endif // TEST_LOGIN_MODE
}

void LoginDialog::setPasswordMode() noexcept
{
    _impl->pswMode = true;
    ui()->userEdit->setReadOnly(true);
    ui()->userEdit->setText(g_globals->session.user);
    ui()->tfaEdit->hide();
    ui()->acceptButton->setText(tr("Verify"));
    ui()->iconLabel->setPixmap(
        QPixmap::fromImage(
            g_globals->session.image.scaled(ui()->iconLabel->size(), Qt::KeepAspectRatio)));
}

void LoginDialog::setNormalMode() noexcept
{
    // Select the image
    loadImage();

    // Determinate if the 2FA is activated
    QSettings settings;
    settings.beginGroup("__Session__data");
    g_globals->session.tfa = settings.value("__2fa__").toBool();
    settings.endGroup();

    ui()->acceptButton->setText(tr("Start"));

    // Index 0 holds the 2FA activated and Index 1 holds the login 2fa
    if (g_globals->session.tfa) {
        _impl->loginTFA = true;
        ui()->tfaEdit->show();
    }
    else
        ui()->tfaEdit->hide();
    ui()->userEdit->setFocus();
}

void LoginDialog::setTFAMode() noexcept
{
    _impl->tfaMode = true;
    ui()->userEdit->setReadOnly(true);
    ui()->userEdit->setText(g_globals->session.user);
    ui()->acceptButton->setText(tr("Verify"));
    ui()->iconLabel->setPixmap(
        QPixmap::fromImage(
            g_globals->session.image.scaled(ui()->iconLabel->size(), Qt::KeepAspectRatio)));

    ui()->userEdit->hide();
    if (!g_globals->session.userTFA.isEmpty())
        ui()->passwordEdit->hide();
}

void LoginDialog::loadImage() noexcept
{
    const QString dataPath      = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    const QString imageFilePath = QString("%1/e30dfd91071c.image.data").arg(dataPath);

    if (QFile::exists(imageFilePath)) {
        g_globals->session.image.load(imageFilePath);

        ui()->iconLabel->setPixmap(
            QPixmap::fromImage(
                g_globals->session.image.scaled(ui()->iconLabel->size(), Qt::KeepAspectRatio)));
    }
}

END_CENTAUR_NAMESPACE
