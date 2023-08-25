/////////////////////////////////////////////////////////////////////////////////////
//
// Created by Ricardo Romero on 12/06/23.
// Copyright (c) 2023 Ricardo Romero.  All rights reserved.
//

#include <QApplication>
#include <QPluginLoader>
#include <ThemeInterface.hpp>

#include "QtCore/qcoreapplication.h"
#include "TestApplication.hpp"
#include <QException>
#include <QFontDatabase>

int main(int argc, char *argv[])
try {
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("CentaurProject");
    QCoreApplication::setOrganizationDomain("centaur.com");
    QCoreApplication::setApplicationName("Centaur-Theme-Test");

    const QString themeLib       = QApplication::applicationDirPath() + "/../lib/libCentTheme.dylib";
    const QString extraLocalPath = QApplication::applicationDirPath() + "/../../../local";

#ifdef Q_OS_MAC
    const QString fontFile = extraLocalPath + "/Inter-VariableFont_slnt,wght.ttf";
#endif

    const auto id = QFontDatabase::addApplicationFont(fontFile);
    if (id != -1) {
        const auto families = QFontDatabase::applicationFontFamilies(id);
        QFont font(families);
        QApplication::setFont(font);
    }
#ifndef QT_NO_DEBUG_OUTPUT
    else {
        qDebug() << "Font could not be loaded";
    }
#endif

    auto loader = std::make_unique<QPluginLoader>(themeLib);

    auto *instance = loader->instance();

    if (auto *themeInstance = qobject_cast<CENTAUR_THEME_INTERFACE_NAMESPACE::ITheme *>(instance);
        themeInstance != nullptr) {
        themeInstance->accessExtra(extraLocalPath);
        auto *style = themeInstance->create("centheme");
        assert(style != nullptr);
        QApplication::setStyle(style);
    }

    auto wnd = std::make_unique<Window>();
    wnd->show();

    return QApplication::exec();
} catch (const QException &ex) {
    qDebug() << ex.what();
}
