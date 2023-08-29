/////////////////////////////////////////////////////////////////////////////////////
//
// Created by Ricardo Romero on 05/06/23.
// Copyright (c) 2023 Ricardo Romero.  All rights reserved.
//

#include <QCoreApplication>
#include <QStandardPaths>
#include <cstdlib>
#include <iostream>

#include "core-naming.hpp"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QCoreApplication::setOrganizationName(cen::defines::_organization_Name);
    QCoreApplication::setOrganizationDomain(cen::defines::_organization_Domain);
    QCoreApplication::setApplicationName(cen::defines::_application_Name);

    const QString ctpath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);

    if (ctpath.isEmpty())
        return EXIT_FAILURE;

    std::cout << ctpath.toStdString();

    return EXIT_SUCCESS;
}
