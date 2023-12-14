//============================================================================
// BSP for DPP example, Microstick II board, preemptive QK kernel, XC32
// Last updated for version 7.3.2
// Last updated on  2023-12-13
//
//                   Q u a n t u m  L e a P s
//                   ------------------------
//                   Modern Embedded Software
//
// Copyright (C) 2005 Quantum Leaps, LLC. <state-machine.com>
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
#include "qpcpp.hpp"             // QP/C++ real-time embedded framework
#include "dpp.hpp"               // DPP Application interface
#include "bsp.hpp"               // Board Support Package

#pragma config  FNOSC    = FRCPLL    // 8 MHz
#pragma config  FPLLIDIV = DIV_2     // 4 MHz
#pragma config  FPLLMUL  = MUL_20    // 80 MHz
#pragma config  FPLLODIV = DIV_2     // 40 MHz == FRC
#pragma config  FWDTEN   = OFF       // watchdog off
#pragma config  FPBDIV   = DIV_1     // same peripheral clock

// #pragma config statements should precede project file includes
#include <xc.h>   // header for PIC32 device in use
#include <sys/attribs.h>

// system clock using FRC and PLL: 40 MHz
#define SYS_FREQ         40000000U
// peripheral clock frequency
#define PER_HZ           (SYS_FREQ / 1U)

// controlling the LED of Microstick II
#define LED_ON()         (LATASET = (1U << 0U))
#define LED_OFF()        (LATACLR = (1U << 0U))
#define LED_TOGGLE()     (LATAINV = (1U << 0U))

namespace { // unnamed namespace for local stuff with internal linkage

Q_DEFINE_THIS_FILE

static std::uint32_t l_rndSeed;

#ifdef Q_SPY
    static QP::QSpyId const l_tickISR {0U};
    static QP::QSpyId const l_testISR {0U};

    enum AppRecords { // application-specific trace records
        PHILO_STAT = QP::QS_USER,
        PAUSED_STAT,
        CONTEXT_SW,
    };

#endif // Q_SPY

} // unnamed namespace

//============================================================================
// Error handler and ISRs...
extern "C" {

Q_NORETURN Q_onError(char const * const module, int_t const id) {
    // NOTE: this implementation of the error handler is intended only
    // for debugging and MUST be changed for deployment of the application
    // (assuming that you ship your production code with assertions enabled).
    Q_UNUSED_PAR(module);
    Q_UNUSED_PAR(id);
    QS_ASSERTION(module, id, 10000U); // report assertion to QS

#ifndef NDEBUG
    // for debugging, hang on in an endless loop...
    for (;;) {
    }
#else
    // perform a system unlock sequence ,starting critical sequence
    SYSKEY = 0x00000000U; //write invalid key to force lock
    SYSKEY = 0xAA996655U; //write key1 to SYSKEY
    SYSKEY = 0x556699AAU; //write key2 to SYSKEY
    // set SWRST bit to arm reset
    RSWRSTSET = 1U;
    // read RSWRST register to trigger reset
    std::uint32_t volatile dummy = RSWRST;
    for (;;) { // explicitly "no-return"
    }
#endif
}
//............................................................................
void assert_failed(char const * const module, int_t const id); // prototype
void assert_failed(char const * const module, int_t const id) {
    Q_onError(module, id);
}

// ISRs --------------------------------------------------------------------

void __ISR(_TIMER_2_VECTOR, IPL4SOFT) tickISR(void) {
    QK_ISR_ENTRY(); // inform QK about the ISR entry

    IFS0CLR = _IFS0_T2IF_MASK; // clear the interrupt source

    QP::QTimeEvt::TICK_X(0U, &l_tickISR); // handle time events at tick rate 0

    QK_ISR_EXIT();  // inform QK about the ISR exit
}
//............................................................................
// for testing interrupt nesting and active object preemption
void __ISR(_EXTERNAL_0_VECTOR, IPL6SOFT) testISR(void) {
    QK_ISR_ENTRY(); // inform QK about the ISR entry

    IFS0CLR = _IFS0_INT0IF_MASK; // clear the interrupt source

    static QP::QEvt const eat_evt(APP::EAT_SIG);
    APP::AO_Table->POST(&eat_evt, &l_testISR);

    QK_ISR_EXIT();  // inform QK about the ISR exit
}

//............................................................................
#ifdef QF_ON_CONTEXT_SW
// NOTE: the context-switch callback is called with interrupts DISABLED
void QF_onContextSw(QP::QActive *prev, QP::QActive *next) {
    QS_BEGIN_INCRIT(CONTEXT_SW, 0U) // in critical section!
        QS_OBJ(prev);
        QS_OBJ(next);
    QS_END_INCRIT()
}
#endif // QF_ON_CONTEXT_SW

} // extern "C"

//============================================================================
namespace BSP {

void init() {
    TRISA = 0x00U; // set LED pins as outputs
    PORTA = 0x00U; // set LED drive state low

    randomSeed(1234U);

    // initialize the QS software tracing
    if (!QS_INIT(nullptr)) {
        Q_ERROR();
    }

    // dictionaries...
    QS_OBJ_DICTIONARY(&l_tickISR);
    QS_OBJ_DICTIONARY(&l_testISR);
    QS_USR_DICTIONARY(PHILO_STAT);
    QS_USR_DICTIONARY(PAUSED_STAT);
    QS_USR_DICTIONARY(CONTEXT_SW);

    QS_ONLY(APP::produce_sig_dict());

    // setup the QS filters...
    QS_GLB_FILTER(QP::QS_ALL_RECORDS);   // all records
    QS_GLB_FILTER(-QP::QS_QF_TICK);      // exclude the clock tick
}
//............................................................................
void start() {
    // initialize event pools
    static QF_MPOOL_EL(APP::TableEvt) smlPoolSto[2*APP::N_PHILO];
    QP::QF::poolInit(smlPoolSto, sizeof(smlPoolSto), sizeof(smlPoolSto[0]));

    // initialize publish-subscribe
    static QP::QSubscrList subscrSto[APP::MAX_PUB_SIG];
    QP::QActive::psInit(subscrSto, Q_DIM(subscrSto));

    // start AOs/threads...
    static QP::QEvt const *philoQueueSto[APP::N_PHILO][10];
    for (std::uint8_t n = 0U; n < APP::N_PHILO; ++n) {
        APP::AO_Philo[n]->start(

            // NOTE: set the preemption-threshold of all Philos to
            // the same level, so that they cannot preempt each other.
            Q_PRIO(n + 3U, APP::N_PHILO + 2U), // QF-prio/pre-thre.

            philoQueueSto[n],        // event queue storage
            Q_DIM(philoQueueSto[n]), // queue length [events]
            nullptr, 0U);            // no stack storage
    }

    static QP::QEvt const *tableQueueSto[APP::N_PHILO];
    APP::AO_Table->start(
        APP::N_PHILO + 7U,           // QP prio. of the AO
        tableQueueSto,               // event queue storage
        Q_DIM(tableQueueSto),        // queue length [events]
        nullptr, 0U);                // no stack storage
}
//............................................................................
void terminate(std::int16_t result) {
    Q_UNUSED_PAR(result);
}
//............................................................................
void displayPhilStat(std::uint8_t const n, char const *stat) {
    Q_UNUSED_PAR(n);

    if (stat[0] == 'e') { // is Philo eating?
        LED_ON();
    }
    else {
        LED_OFF();
    }

    // app-specific record
    QS_BEGIN_ID(PHILO_STAT, APP::AO_Table->getPrio())
        QS_U8(1, n);  // Philosopher number
        QS_STR(stat); // Philosopher status
    QS_END()
}
//............................................................................
void displayPaused(std::uint8_t const paused) {
    Q_UNUSED_PAR(paused);

    // application-specific trace record
    QS_BEGIN_ID(PAUSED_STAT, APP::AO_Table->getPrio())
        QS_U8(1, paused);  // Paused status
    QS_END()
}
//............................................................................
void randomSeed(std::uint32_t seed) {
    l_rndSeed = seed;
}
//............................................................................
std::uint32_t random() { // a cheap pseudo-random-number generator

    QP::QSchedStatus lockStat = QP::QK::schedLock(APP::N_PHILO);
    // "Super-Duper" Linear Congruential Generator (LCG)
    // LCG(2^32, 3*7*11*13*23, 0, seed)
    std::uint32_t rnd = l_rndSeed * (3U*7U*11U*13U*23U);
    l_rndSeed = rnd; // set for the next time
    QP::QK::schedUnlock(lockStat);

    return (rnd >> 8U);
}

} // namespace BSP


//===========================================================================
namespace QP {

void QF::onStartup() {
    INTCONSET = _INTCON_MVEC_MASK; // configure multi-vectored interrupts

    T2CON = 0x0060U; // stop timer, set up for 1:64 prescaler
    TMR2 = 0U;       // count from zero up to the period
    PR2 = SYS_FREQ / (BSP::TICKS_PER_SEC * 64U); // set the Timer2 period
    IFS0CLR = _IFS0_T2IF_MASK;   // clear Timer2 Interrupt Flag
    IEC0SET = _IEC0_T2IE_MASK;   // enable Timer2 interrupt
    T2CONSET = _T2CON_ON_MASK;   // Start Timer2

    INTCONbits.INT0EP = 1U;      // INT0 interrupt on positive edge
    IEC0SET = _IEC0_INT0IE_MASK; // enable INT0 interrupt
    IFS0CLR = _IFS0_INT0IF_MASK; // clear the interrupt for INT0

    // explicitly assign priorities to all interrupts...
    // NOTE: must match the IPLxSOFT settings in the __ISR() macros
    IPC2bits.T2IP   = 4U; // Timer2 interrupt priority, must match tickISR
    IPC0bits.INT0IP = 6U; // set INT0 priority; must match IPL in testISR
}
//............................................................................
void QF::onCleanup() {
}
//............................................................................
void QK::onIdle() {

    // NOTE: not enough LEDs on the Microstick II board to implement
    // the idle loop activity indicator ...
    //QF_INT_DISABLE();
    //LED_ON ();
    //LED_OFF();
    //QF_INT_ENABLE();

#ifdef Q_SPY
    while (U2STAbits.UTXBF == 0U) { // TX Buffer not full?
        QF_INT_DISABLE();
        std::uint16_t b = QS::getByte();
        QF_INT_ENABLE();

        if (b != QS_EOD) { // End-Of-Data reached?
            U2TXREG = b; // stick the byte to TXREG for transmission
        }
        else {
            break; // break out of the loop
        }
    }
#elif defined NDEBUG
    _wait();   // execute the WAIT instruction to stop the CPU
#endif
}

//============================================================================
// QS callbacks...
#ifdef Q_SPY

#define QS_BUF_SIZE   4096U
#define QS_BAUD_RATE  115200U

//............................................................................
bool QS::onStartup(void const *arg) {
    Q_UNUSED_PAR(arg);

    static uint8_t qsBuf[QS_BUF_SIZE]; // buffer for QS-TX
    initBuf(qsBuf, sizeof(qsBuf)); // initialize the QS-TX channel

    //TBD: implement QS-RX channel

    // initialize the UART2 for transmitting the QS trace data
    U2RXRbits.U2RXR = 3;    // Set U2RX to RPB11, pin 22, J6-5
    RPB10Rbits.RPB10R = 2;  // Set U2TX to RPB10, pin 21, J6-4

    U2STA  = 0x0000U;       // use default settings of 8N1
    U2MODE = 0x0008U;       // enable high baud rate
    U2BRG  = (uint16_t)((PER_HZ / (4.0 * QS_BAUD_RATE)) - 1.0 + 0.5);
    U2MODEbits.UARTEN = 1;
    U2STAbits.UTXEN   = 1;

    return true; // return success
}
//............................................................................
void QS::onCleanup() {
}
//............................................................................
// NOTE:
// No critical section in QS::onFlush() to avoid nesting of critical sections
// in case QS::onFlush() is called from Q_onError().
void QS::onFlush(void) {
    for (;;) {
        std::uint16_t b = getByte();
        if (b != QS_EOD) {
            while (U2STAbits.UTXBF) { // TX Buffer full?
            }
            U2TXREG = b; // stick the byte to TXREG for transmission
        }
        else {
            break;
        }
    }
}
//............................................................................
QSTimeCtr QS::onGetTime() {
    return __builtin_mfc0(_CP0_COUNT, _CP0_COUNT_SELECT);
}
//............................................................................
void QS::onReset() {
    // perform a system unlock sequence ,starting critical sequence
    SYSKEY = 0x00000000; //write invalid key to force lock
    SYSKEY = 0xAA996655; //write key1 to SYSKEY
    SYSKEY = 0x556699AA; //write key2 to SYSKEY
    // set SWRST bit to arm reset
    RSWRSTSET = 1;
    // read RSWRST register to trigger reset
    std::uint32_t volatile dummy = RSWRST;
    // prevent any unwanted code execution until reset occurs
}
//............................................................................
void QS::onCommand(std::uint8_t cmdId, std::uint32_t param1,
                   std::uint32_t param2, std::uint32_t param3)
{
    Q_UNUSED_PAR(cmdId);
    Q_UNUSED_PAR(param1);
    Q_UNUSED_PAR(param2);
    Q_UNUSED_PAR(param3);
}

#endif // Q_SPY
//----------------------------------------------------------------------------

} // namespace QP

//============================================================================
// NOTE2:
// The Temporal Proximity Timer is used to prevent a race condition of
// servicing an interrupt after re-enabling interrupts and before executing
// the WAIT instruction. The Proximity Timer is enabled for all interrupt
// priority levels (see QF_onStartup()). The Proximity Timer is set to 4
// CPU clocks right before re-enabling interrupts (with the DI instruction)
// The 4 clock ticks should be enough to execute the (DI,WAIT) instruction
// pair _atomically_.
//

