/////////////////////////////////////////////////////////////////////////////////////
//
// Created by Ricardo Romero on 06/12/21.
// Copyright (c) 2021 Ricardo Romero.  All rights reserved.
//

#include "CentaurApp.hpp"
#include <QApplication>
#include <QFontDatabase>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName("CentaurProject");
    QCoreApplication::setOrganizationDomain("centaur.com");
    QCoreApplication::setApplicationName("Centaur");

#ifdef Q_OS_MAC
    const QString fontFile = QCoreApplication::applicationDirPath() + "/../Resources/Fonts/Inter-VariableFont_slnt,wght.ttf";
#endif

    const auto id = QFontDatabase::addApplicationFont(fontFile);
    if (id != -1)
    {
        const auto families = QFontDatabase::applicationFontFamilies(id);
        QFont font(families);
        QApplication::setFont(font);
    }
#ifndef QT_NO_DEBUG_OUTPUT
    else
    {
        qDebug() << "Font could not be loaded";
    }
#endif

    auto app = std::make_unique<CENTAUR_NAMESPACE::CentaurApp>();
    app->show();

    return QApplication::exec();
}
