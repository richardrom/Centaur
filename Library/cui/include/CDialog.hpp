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

class CENT_LIBRARY CDialog : public QDialog
{
public:
    explicit CDialog(QWidget *parent = nullptr);
    ~CDialog() override;

    /// \brief Connects the button release signal to the onAccept slot
    /// \param button Button that will generate the release signal
    void setAccept(QPushButton *button) const noexcept;

protected slots:
    /// \brief Stores the saved dialog state and emits the accept signal.
    virtual void onAccept() noexcept;

protected:
    /// \brief Load the store dialog state
    /// \note The dialog name will be used to store the data. Having an empty object name will assert the execution
    void restoreInterface() noexcept;
    /// \brief Save the store dialog state
    /// \note The dialog name will be used to store the data. Having an empty object name will assert the execution
    void saveInterface() noexcept;

    void paintEvent(QPaintEvent *event) override;

private:
    C_P_IMPL()
};

END_CENTAUR_NAMESPACE

#endif // CENTAUR_CTDIALOG_HPP
