/////////////////////////////////////////////////////////////////////////////////////
//
// Created by Ricardo Romero on 12/06/23.
// Copyright (c) 2023 Ricardo Romero.  All rights reserved.
//

#include <QApplication>
#include <QPluginLoader>
#include <ThemeInterface.hpp>

#include "TestApplication.hpp"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("CentaurProject");
    QCoreApplication::setOrganizationDomain("centaur.com");
    QCoreApplication::setApplicationName("Centaur-Theme-Test");

    const QString themeLib       = QApplication::applicationDirPath() + "/../lib/libCentTheme.dylib";
    const QString extraLocalPath = QApplication::applicationDirPath() + "/../../../local";

    auto loader = std::make_unique<QPluginLoader>(themeLib);

    auto instance = loader->instance();

    if (auto *themeInstance = qobject_cast<CENTAUR_THEME_INTERFACE_NAMESPACE::ITheme *>(instance);
        themeInstance != nullptr)
    {
        themeInstance->accessExtra(extraLocalPath);
        auto *style = themeInstance->create("centheme");
        assert(style != nullptr);
        QApplication::setStyle(style);
    }

    auto wnd = std::make_unique<Window>();
    wnd->show();

    return QApplication::exec();
}
