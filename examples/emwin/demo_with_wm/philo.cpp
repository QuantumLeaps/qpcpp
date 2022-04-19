//============================================================================
// Product: DPP example
// Last updated for version 6.8.0
// Last updated on  2020-01-22
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2005-2020 Quantum Leaps, LLC. All rights reserved.
//
// This program is open source software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Alternatively, this program may be distributed and modified under the
// terms of Quantum Leaps commercial licenses, which expressly supersede
// the GNU General Public License and are specifically designed for
// licensees interested in retaining the proprietary status of their code.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <www.gnu.org/licenses/>.
//
// Contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
#include "qpcpp.hpp"
#include "dpp.hpp"
#include "bsp.hpp"

Q_DEFINE_THIS_FILE

// Active object class -------------------------------------------------------
class Philo : public QActive {
private:
    QTimeEvt m_timeEvt; // to timeout thinking or eating

public:
    Philo();

private:
    Q_STATE_DECL(initial);
    Q_STATE_DECL(thinking);
    Q_STATE_DECL(hungry);
    Q_STATE_DECL(eating);
};

// Local objects -------------------------------------------------------------
static Philo l_philo[N_PHILO]; // storage for all Philos

#define THINK_TIME  7
#define EAT_TIME    5

// helper macro to provide the ID of Philo "me_"
#define PHILO_ID(me_)    ((uint8_t)((me_) - l_philo))

enum InternalSignals { // internal signals
    TIMEOUT_SIG = MAX_SIG
};
// Global objects ------------------------------------------------------------
QActive * const AO_Philo[N_PHILO] = { // "opaque" pointers to Philo AOs
    &l_philo[0],
    &l_philo[1],
    &l_philo[2],
    &l_philo[3],
    &l_philo[4]
};

//............................................................................
Philo::Philo()
    : QActive((QStateHandler)&Philo::initial),
      m_timeEvt(this, TIMEOUT_SIG)
{}
//............................................................................
Q_STATE_DEF(Philo, initial) {
    subscribe(EAT_SIG);

    return tran(&thinking);
}
//............................................................................
Q_STATE_DEF(Philo, thinking) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            m_timeEvt.armX(THINK_TIME);
            return Q_RET_HANDLED;
        }
        case TIMEOUT_SIG: {
            return tran(&hungry);
        }
        case EAT_SIG: // intentionally fall-through
        case DONE_SIG: {
            // EAT or DONE must be for other Philos than this one
            Q_ASSERT(Q_EVT_CAST(TableEvt)->philoNum != PHILO_ID(this));
            return Q_RET_HANDLED;
        }
    }
    return super(&top);
}
//............................................................................
Q_STATE_DEF(Philo, hungry) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            TableEvt *pe = Q_NEW(TableEvt, HUNGRY_SIG);
            pe->philoNum = PHILO_ID(this);
            AO_Table->POST(pe, this);
            return Q_RET_HANDLED;
        }
        case EAT_SIG: {
            if (((TableEvt *)e)->philoNum == PHILO_ID(this)) {
                return tran(&eating);
            }
            break;
        }
        case DONE_SIG: {
            // DONE must be for other Philos than this one
            Q_ASSERT(Q_EVT_CAST(TableEvt)->philoNum != PHILO_ID(this));
            return Q_RET_HANDLED;
        }
    }
    return super(&top);
}
//............................................................................
Q_STATE_DEF(Philo, eating) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            m_timeEvt.armX(EAT_TIME);
            return Q_RET_HANDLED;
        }
        case Q_EXIT_SIG: {
            TableEvt *pe = Q_NEW(TableEvt, DONE_SIG);
            pe->philoNum = PHILO_ID(this);
            QF::PUBLISH(pe, this);
            return Q_RET_HANDLED;
        }
        case TIMEOUT_SIG: {
            return tran(&thinking);
        }
        case EAT_SIG: // intentionally fall-through
        case DONE_SIG: {
            // EAT or DONE must be for other Philos than this one
            Q_ASSERT(Q_EVT_CAST(TableEvt)->philoNum != PHILO_ID(this));
            return Q_RET_HANDLED;
        }
    }
    return super(&top);
}

