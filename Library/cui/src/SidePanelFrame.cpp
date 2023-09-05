/////////////////////////////////////////////////////////////////////////////////////
//
// Created by Ricardo Romero on 04/09/23.
// Copyright (c) 2023 Ricardo Romero.  All rights reserved.
//

/////////////////////////////////////////////////////////////////////////////////////
//
// Created by Ricardo Romero on 03/09/23.
// Copyright (c) 2023 Ricardo Romero.  All rights reserved.
//

#include "SidePanelFrame.hpp"
#include "cui.hpp"
#include <QPaintEvent>
#include <QPainter>
#include <ThemeInterface.hpp>

BEGIN_CENTAUR_NAMESPACE

struct SidePanelFrame::Impl
{
    const CENTAUR_THEME_INTERFACE_NAMESPACE::SideFrameInformation *uiInformation { nullptr };
};

SidePanelFrame::SidePanelFrame(QWidget *parent) :
    QFrame(parent),
    _impl { std::make_unique<SidePanelFrame::Impl>() }
{
}

SidePanelFrame::~SidePanelFrame() = default;

void SidePanelFrame::paintEvent(QPaintEvent *event)
{
    if (cuiTheme() == nullptr) {
        QFrame::paintEvent(event);
        return;
    }
    if (P_IMPL()->uiInformation == nullptr) {
        P_IMPL()->uiInformation = std::addressof(cuiTheme()->uiElements().sideFrameInformation);
        assert(P_IMPL()->uiInformation != nullptr);
    }

    QPainter painter(this);
    painter.fillRect(rect(), P_IMPL()->uiInformation->backgroundBrush);
}

END_CENTAUR_NAMESPACE
