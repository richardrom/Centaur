/////////////////////////////////////////////////////////////////////////////////////
//
// Created by Ricardo Romero on 04/06/23.
// Copyright (c) 2023 Ricardo Romero.  All rights reserved.
//

#include "CDialog.hpp"
#include <QPushButton>
#include <QSettings>

BEGIN_CENTAUR_NAMESPACE

CDialog::CDialog(QWidget *parent) :
    QDialog { parent }
{
#ifdef Q_OS_MAC
    setWindowFlag(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
#endif

    restoreInterface();
}

CDialog::~CDialog() = default;

void CDialog::onAccept() noexcept
{
    saveInterface();
    accept();
}

void CDialog::restoreInterface() noexcept
{
    const QString &objectName = this->objectName();
    assert(!objectName.isEmpty());

    QSettings settings("CentaurProject", "Centaur");
    settings.beginGroup(objectName);
    restoreGeometry(settings.value("geometry").toByteArray());
    settings.endGroup();
}

void CDialog::setAccept(QPushButton *button) const noexcept
{
    connect(button, &QPushButton::released, this, &CDialog::onAccept);
}

void CDialog::saveInterface() noexcept
{
    const QString &objectName = this->objectName();
    assert(!objectName.isEmpty());

    QSettings settings("CentaurProject", "Centaur");
    settings.beginGroup(objectName);
    settings.setValue("geometry", saveGeometry());
    settings.endGroup();
}

END_CENTAUR_NAMESPACE
