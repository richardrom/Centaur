/////////////////////////////////////////////////////////////////////////////////////
//
// Created by Ricardo Romero on 29/08/23.
// Copyright (c) 2023 Ricardo Romero.  All rights reserved.
//

#pragma once

#ifndef __cplusplus
#error "C++ compiler needed"
#endif /*__cplusplus*/

#ifndef CENTAUR_WINDOWFRAME_HPP
#define CENTAUR_WINDOWFRAME_HPP

#include "Centaur.hpp"
#include <QFrame>

BEGIN_CENTAUR_NAMESPACE

/// \brief A resizable and moveable frame
class CENT_LIBRARY WindowFrame : public QFrame
{
    Q_OBJECT
public:
    enum class FrameMode
    {
        Resizable,
        Movable,
        AllModes
    };

    explicit WindowFrame(QWidget *parent = nullptr) noexcept;
    WindowFrame(FrameMode mode = FrameMode::AllModes, QWidget *parent = nullptr) noexcept;
    ~WindowFrame() override;

    /// \brief Set the widget that is going to be movable
    /// \param widget Widget that is going to be move
    /// \note By default, the direct parent of the WindowFrame is the one that is going to be moved
    auto overrideMovableParent(QWidget *widget) -> void;
    /// \brief Set the widget that is going to be movable. This override will find the farthest parent that is not nullptr
    /// \return The parent found
    auto overrideMovableParent() -> QWidget *;

    /// \brief Get the movable parent
    /// \return A widget that is receiving all the resizing and moving events
    C_NODISCARD QWidget *activeParent() const;

    /// \brief Obtain the current frame mode
    /// \return The current frame mode
    WindowFrame::FrameMode frameMode() const;

protected:
    void leaveEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

    C_P_IMPL()
};

END_CENTAUR_NAMESPACE

#endif // CENTAUR_WINDOWFRAME_HPP
