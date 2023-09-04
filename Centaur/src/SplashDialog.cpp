/////////////////////////////////////////////////////////////////////////////////////
//
// Created by Ricardo Romero on 12/11/22.
// Copyright (c) 2022 Ricardo Romero.  All rights reserved.
//

#include "SplashDialog.hpp"
#include "../ui/ui_SplashDialog.h"
#include <QScreen>
#include <QSettings>

BEGIN_CENTAUR_NAMESPACE

struct SplashDialog::Impl
{
    inline Impl() :
        ui { new Ui::SplashDialog } { }

    inline ~Impl() = default;

    std::unique_ptr<Ui::SplashDialog> ui;
};

SplashDialog::SplashDialog(QWidget *parent) :
    QDialog { parent },
    _impl { new Impl }
{
#ifdef Q_OS_MAC
    setWindowFlag(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
#endif

    ui()->setupUi(this);
    ui()->logMainFrame->overrideParent(this);

    // Center the dialog
    const QRect screenGeometry = QGuiApplication::screens().at(0)->geometry();

    const int xCenter = (screenGeometry.width() - this->width()) / 2;
    const int yCenter = (screenGeometry.height() - this->height()) / 2;
    this->move(xCenter, yCenter);
}

SplashDialog::~SplashDialog() = default;

Ui::SplashDialog *SplashDialog::ui()
{
    return _impl->ui.get();
}

/**
 * Since this dialog is only intended to be shown in the startup of the code
 * QCoreApplication::processEvents();
 * is mandatory to display all changes
 */

void SplashDialog::setDisplayText(const QString &text)
{
    ui()->displayText->setText(text);
    ui()->displayText->repaint();
    QCoreApplication::processEvents();
}

void SplashDialog::setProgressRange(int min, int max)
{
    ui()->initializationProgressBar->setRange(min, max);
    QCoreApplication::processEvents();
}

void SplashDialog::setProgressPos(int pos)
{
    ui()->initializationProgressBar->setValue(pos);
    QCoreApplication::processEvents();
}

void SplashDialog::step()
{
    auto pos = ui()->initializationProgressBar->value();
    setProgressPos(pos + 1);
}

std::pair<int, int> SplashDialog::getProgressRange()
{
    return { ui()->initializationProgressBar->minimum(),
        ui()->initializationProgressBar->maximum() };
}

END_CENTAUR_NAMESPACE
