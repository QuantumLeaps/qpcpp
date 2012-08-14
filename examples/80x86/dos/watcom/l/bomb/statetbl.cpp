//////////////////////////////////////////////////////////////////////////////
// Product: Generic State-Table event processor
// Last Updated for Version: 4.0.00
// Date of the Last Update:  Apr 07, 2008
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2002-2008 Quantum Leaps, LLC. All rights reserved.
//
// This software may be distributed and modified under the terms of the GNU
// General Public License version 2 (GPL) as published by the Free Software
// Foundation and appearing in the file GPL.TXT included in the packaging of
// this file. Please note that GPL Section 2[b] requires that all works based
// on this software must also be made publicly available under the terms of
// the GPL ("Copyleft").
//
// Alternatively, this software may be distributed and modified under the
// terms of Quantum Leaps commercial licenses, which expressly supersede
// the GPL and are specifically designed for licensees interested in
// retaining the proprietary status of their code.
//
// Contact information:
// Quantum Leaps Web site:  http://www.quantum-leaps.com
// e-mail:                  info@quantum-leaps.com
//////////////////////////////////////////////////////////////////////////////
#include "bsp.h"             // for the C99-standard exact-width integer types
#include "statetbl.h"

                 // see also embedded-systems-friendly assertions in Chapter 6
#include <assert.h>

//............................................................................
void StateTable::init(void) {
    (*m_initial)(this, (Event *)0);             // top-most initial transition

    assert(m_state < m_nStates);        // the initial tran. must change state
}
//............................................................................
void StateTable::dispatch(Event const *e) {
    assert(e->sig < m_nSignals);                // require the signal in range

    Tran t = m_state_table[m_state*m_nSignals + e->sig];
    (*t)(this, e);                          // execute the transition function

    assert(m_state < m_nStates);           // ensure that state stays in range
}
//............................................................................
void StateTable::empty(StateTable *, Event const *) {
}
