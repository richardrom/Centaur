/////////////////////////////////////////////////////////////////////////////////////
//
// Created by Ricardo Romero on 28/08/23.
// Copyright (c) 2023 Ricardo Romero.  All rights reserved.
//
#include "cui.hpp"

namespace
{
    CENTAUR_THEME_INTERFACE_NAMESPACE::ITheme *_theme { nullptr };
}

void CENTAUR_NAMESPACE::setCUITheme(CENTAUR_THEME_INTERFACE_NAMESPACE::ITheme *theme) noexcept
{
    _theme = theme;
}

CENTAUR_THEME_INTERFACE_NAMESPACE::ITheme *CENTAUR_NAMESPACE::cuiTheme() noexcept
{
    return _theme;
}
