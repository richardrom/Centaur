/////////////////////////////////////////////////////////////////////////////////////
//
// Created by Ricardo Romero on 28/08/23.
// Copyright (c) 2023 Ricardo Romero.  All rights reserved.
//

#pragma once

#ifndef __cplusplus
#error "C++ compiler needed"
#endif /*__cplusplus*/

#ifndef CENTAUR_CUI_HPP
#define CENTAUR_CUI_HPP

#include "Centaur.hpp"
#include <QDialog>
#include <ThemeInterface.hpp>

BEGIN_CENTAUR_NAMESPACE

CENT_LIBRARY void setCUITheme(CENTAUR_THEME_INTERFACE_NAMESPACE::ITheme *theme) noexcept;

CENT_LIBRARY CENTAUR_THEME_INTERFACE_NAMESPACE::ITheme *cuiTheme() noexcept;

END_CENTAUR_NAMESPACE

#endif // CENTAUR_CUI_HPP
