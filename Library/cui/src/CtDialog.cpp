/////////////////////////////////////////////////////////////////////////////////////
//
// Created by Ricardo Romero on 04/06/23.
// Copyright (c) 2023 Ricardo Romero.  All rights reserved.
//

#include "CtDialog.hpp"
#include <QPushButton>
#include <QSettings>

BEGIN_CENTAUR_NAMESPACE

CtDialog::CtDialog(QWidget *parent) :
    QDialog { parent }
{
#ifdef Q_OS_MAC
    setWindowFlag(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
#endif

    restoreInterface();
}

CtDialog::~CtDialog() = default;

void CtDialog::onAccept() noexcept
{
    QSettings settings("CentaurProject", "Centaur");
    settings.beginGroup("CtDialog");
    settings.setValue("geometry", saveGeometry());
    settings.endGroup();

    accept();
}

void CtDialog::restoreInterface() noexcept
{
    QSettings settings("CentaurProject", "Centaur");
    settings.beginGroup("CtDialog");
    restoreGeometry(settings.value("geometry").toByteArray());
    settings.endGroup();
}

void CtDialog::setAccept(QPushButton *button) const noexcept
{
    connect(button, &QPushButton::released, this, &CtDialog::onAccept);
}

END_CENTAUR_NAMESPACE
