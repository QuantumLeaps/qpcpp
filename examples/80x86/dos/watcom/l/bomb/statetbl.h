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
#ifndef statetbl_h
#define statetbl_h

struct Event {
    uint16_t sig;                                       // signal of the event
    // add event parameters by subclassing the Event structure...
};

class StateTable;                                       // forward declaration
typedef void (*Tran)(StateTable *me, Event const *e);   // pointer-to-function

class StateTable {
public:
    void init(void);
    void dispatch(Event const *e);
    static void empty(StateTable *me, Event const *e);

protected:
              // protected ctor prevents direct instantiation (abstract class)
    StateTable(Tran const *table, uint8_t n_states, uint8_t n_signals,
               Tran initial)
       : m_state(n_states),
         m_state_table(table), m_nStates(n_states), m_nSignals(n_signals),
         m_initial(initial) {}

    void tran(uint8_t target) { m_state = target; }

private:
    uint8_t m_state;                               // the current active state
    Tran const *m_state_table;                              // the State-Table
    uint8_t m_nStates;                                     // number of states
    uint8_t m_nSignals;                                   // number of signals
    Tran m_initial;                                  // the initial transition
};

#define TRAN(target_) (me->tran(target_))

#endif                                                           // statetbl_h
