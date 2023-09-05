/////////////////////////////////////////////////////////////////////////////////////
//
// Created by Ricardo Romero on 03/09/23.
// Copyright (c) 2023 Ricardo Romero.  All rights reserved.
//

#include "CommandsFrame.hpp"
#include "cui.hpp"
#include <QPaintEvent>
#include <QPainter>
#include <ThemeInterface.hpp>

BEGIN_CENTAUR_NAMESPACE

struct CommandsFrame::Impl
{
    const CENTAUR_THEME_INTERFACE_NAMESPACE::CommandFrameInformation *uiInformation { nullptr };
};

CommandsFrame::CommandsFrame(QWidget *parent) :
    QFrame(parent),
    _impl { std::make_unique<CommandsFrame::Impl>() }
{
}

CommandsFrame::~CommandsFrame() = default;

void CommandsFrame::paintEvent(QPaintEvent *event)
{
    if (cuiTheme() == nullptr) {
        QFrame::paintEvent(event);
        return;
    }
    if (P_IMPL()->uiInformation == nullptr) {
        P_IMPL()->uiInformation = std::addressof(cuiTheme()->uiElements().commandFrameInformation);
        assert(P_IMPL()->uiInformation != nullptr);
    }

    QPainter painter(this);
    painter.fillRect(rect(), P_IMPL()->uiInformation->backgroundBrush);
}

END_CENTAUR_NAMESPACE
