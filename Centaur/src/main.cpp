/////////////////////////////////////////////////////////////////////////////////////
//
// Created by Ricardo Romero on 06/12/21.
// Copyright (c) 2021 Ricardo Romero.  All rights reserved.
//

#include "CentaurApp.hpp"
#include "core-naming.hpp"
#include <QApplication>
#include <QFontDatabase>
#include <QPluginLoader>
#include <QSettings>
#include <ThemeInterface.hpp>
#include <cui.hpp>

int main(int argc, char *argv[])
{
    const QApplication qApplication(argc, argv);

    QCoreApplication::setOrganizationName(cen::defines::_organization_Name);
    QCoreApplication::setOrganizationDomain(cen::defines::_organization_Domain);
    QCoreApplication::setApplicationName(cen::defines::_application_Name);

#ifdef Q_OS_MAC
    const QString defaultThemeLibrary { "libCentTheme.dylib" };
    const QString fontFile = QCoreApplication::applicationDirPath() + "/../Resources/Fonts/Inter-VariableFont_slnt,wght.ttf";
#elif Q_OS_WIN
    const QString defaultThemeLibrary { "libCentTheme.dll" };
    // TODO: Load font in windows
#elif Q_OS_LINUX
    const QString defaultThemeLibrary { "libCentTheme.so" };
    // TODO: Load font in Linux
#endif

    const auto applicationFont = QFontDatabase::addApplicationFont(fontFile);
    if (applicationFont != -1) {
        const auto families = QFontDatabase::applicationFontFamilies(applicationFont);
        const QFont font(families);
        QApplication::setFont(font);
    }
#ifndef QT_NO_DEBUG_OUTPUT
    else {
        qDebug() << "Font could not be loaded";
    }
#endif

    const QString appLocalDataLocation = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    const QString themePath            = QString(QDir::toNativeSeparators("%1/theme")).arg(appLocalDataLocation);
    const QString extraPath            = QString(QDir::toNativeSeparators("%1/Plugins/extra")).arg(appLocalDataLocation);

    // Explore the themes path in search of
    QSettings settings;
    settings.beginGroup("application-theme");
    const auto lastLoadedThemeLibrary = settings.value("theme-library", defaultThemeLibrary).toString();
    const auto lastLoadedThemeName    = settings.value("theme-name-implementation", "centheme").toString();
    settings.endGroup();

    const QString themeLibraryFile = QString("%1/%2").arg(themePath, lastLoadedThemeLibrary);

    if (QFile::exists(themeLibraryFile)) {

        auto loader = std::make_unique<QPluginLoader>("/Volumes/RicardoESSD/Projects/Centaur/build/debug/lib/libCentTheme.dylib");
        // auto loader = std::make_unique<QPluginLoader>(themeLibraryFile);

        auto *instance = loader->instance();

        if (auto *themeInstance = qobject_cast<CENTAUR_THEME_INTERFACE_NAMESPACE::ITheme *>(instance);
            themeInstance != nullptr) {
            themeInstance->accessExtra(extraPath);
            auto *style = themeInstance->create(lastLoadedThemeName);
            assert(style != nullptr);
            QApplication::setStyle(style);
            // Set the CUI internal theme
            CENTAUR_NAMESPACE::setCUITheme(themeInstance);
        }
        else
            qWarning() << "The theme library was not loaded. Default theme will loaded";
    }
    else {
        qWarning() << "The last loaded theme library was not found. Default theme will loaded";
    }

    auto app = std::make_unique<CENTAUR_NAMESPACE::CentaurApp>();
    app->show();

    return QApplication::exec();
}
