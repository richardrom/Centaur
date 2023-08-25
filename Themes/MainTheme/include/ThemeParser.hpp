/////////////////////////////////////////////////////////////////////////////////////
//
// Created by Ricardo Romero on 05/06/23.
// Copyright (c) 2023 Ricardo Romero.  All rights reserved.
//

#pragma once

#include "QtGui/qpainter.h"
#ifndef __cplusplus
#error "C++ compiler needed"
#endif /*__cplusplus*/

#ifndef CENTAUR_THEMEPARSER_HPP
#define CENTAUR_THEMEPARSER_HPP

#include <QBrush>
#include <QColor>
#include <QPen>
#include <QString>

#include <string>
#include <unordered_map>

#include <Centaur.hpp>
#include <ThemeInterface.hpp>

#ifdef USE_THEME_TESTING
#ifdef THEME_USE_EXPORTS
#ifdef _WIN32
#ifdef C_MSVC
#define THEME_LIBRARY __declspec(dllexport) #else
#define THEME_LIBRARY __attribute__((dllexport))
#endif
#else
#define THEME_LIBRARY __attribute__((visibility("default")))
#endif
#else
#ifdef _WIN32
#ifdef C_MSVC
#define THEME_LIBRARY
#else
#define THEME_LIBRARY
#endif
#else
#define THEME_LIBRARY __attribute__((visibility("default")))
#endif
#endif /*THEME_USE_EXPORTS*/
#else
#define THEME_LIBRARY __attribute__((visibility("hidden")))
#endif /*USE_THEME_TESTING*/

namespace theme
{
    struct THEME_LIBRARY ThemeParser final
    {
        ThemeParser();
        ~ThemeParser();

        void loadTheme(const std::string &file);

    public:
        QStringList getErrors();

    public:
        std::string themeScheme;
        QPainter::RenderHints renderHints { 0 };

        CENTAUR_THEME_INTERFACE_NAMESPACE::ThemeConstants constants;
        CENTAUR_THEME_INTERFACE_NAMESPACE::ColorScheme scheme;
        CENTAUR_THEME_INTERFACE_NAMESPACE::UIElements uiElements;

    private:
        C_P_IMPL()
    };
} // namespace theme

#endif // CENTAUR_THEMEPARSER_HPP
