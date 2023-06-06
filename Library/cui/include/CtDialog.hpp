/////////////////////////////////////////////////////////////////////////////////////
//
// Created by Ricardo Romero on 04/06/23.
// Copyright (c) 2023 Ricardo Romero.  All rights reserved.
//

#pragma once

#ifndef __cplusplus
#error "C++ compiler needed"
#endif /*__cplusplus*/

#ifndef CENTAUR_CTDIALOG_HPP
#define CENTAUR_CTDIALOG_HPP

#include "Centaur.hpp"
#include <QDialog>

BEGIN_CENTAUR_NAMESPACE

class CtDialog : public QDialog
{
public:
    explicit CtDialog(QWidget *parent = nullptr);
    ~CtDialog() override;

public:
    /**
     * \brief Connects the button release signal to the onAccept slot
     * \param button Button that will generate the release signal
     */
    void setAccept(QPushButton *button) const noexcept;

protected slots:
    /**
     * \brief Stores the saved dialog state
     */
    virtual void onAccept() noexcept;

protected:
    /**
     * \brief Load the store dialog state
     */
    virtual void restoreInterface() noexcept;
};

END_CENTAUR_NAMESPACE

#endif // CENTAUR_CTDIALOG_HPP
