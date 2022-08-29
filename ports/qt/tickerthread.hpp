//============================================================================
// Copyright (C) 2005 Quantum Leaps, LLC <state-machine.com>.
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial
//
// This software is dual-licensed under the terms of the open source GNU
// General Public License version 3 (or any later version), or alternatively,
// under the terms of one of the closed source Quantum Leaps commercial
// licenses.
//
// The terms of the open source GNU General Public License version 3
// can be found at: <www.gnu.org/licenses/gpl-3.0>
//
// The terms of the closed source Quantum Leaps commercial licenses
// can be found at: <www.state-machine.com/licensing>
//
// Redistributions in source code must retain this top-level comment block.
// Plagiarizing this software to sidestep the license obligations is illegal.
//
// Contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
//! @date Last updated on: 2022-08-25
//! @version Last updated for: @ref qpcpp_7_1_0
//!
//! @file
//! @brief QP/C++ port to Qt

#ifndef TICKERTHREAD_HPP
#define TICKERTHREAD_HPP

#include <QThread>

namespace QP {

class TickerThread : public QThread {
    Q_OBJECT

public:
    TickerThread() : m_isRunning(false) {}
    virtual ~TickerThread();
    virtual void run();

public:
    unsigned m_tickInterval;
    bool     m_isRunning;
};

} // namespace QP

#endif // TICKERTHREAD_HPP
