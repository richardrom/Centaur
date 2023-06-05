/////////////////////////////////////////////////////////////////////////////////////
//
// Created by Ricardo Romero on 05/06/23.
// Copyright (c) 2023 Ricardo Romero.  All rights reserved.
//

#include <QCoreApplication>
#include <QStandardPaths>
#include <cstdlib>
#include <iostream>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QCoreApplication::setOrganizationName("CentaurProject");
    QCoreApplication::setOrganizationDomain("centaur.com");
    QCoreApplication::setApplicationName("Centaur");

    const QString ctpath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);

    if (ctpath.isEmpty())
        return EXIT_FAILURE;

    std::cout << ctpath.toStdString();

    return EXIT_SUCCESS;
}
