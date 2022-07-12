//============================================================================
// Product: DPP example, LAUCHXL2-TMS570LS12 board, preemptive QK kernel
// Last updated for version 6.9.3
// Last updated on  2021-03-03
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// Copyright (C) 2005-2021 Quantum Leaps. All rights reserved.
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
// along with this program. If not, see <www.gnu.org/licenses>.
//
// Contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
#include "qpcpp.hpp"
#include "dpp.hpp"
#include "bsp.hpp"

#include "sys_common.h"
#include "sys_core.h"
#include "sys_vim.h"
#include "system.h"
#include "gio.h"
#include "rti.h"
#include "het.h"
#include "sci.h"
// add other drivers if necessary...

Q_DEFINE_THIS_FILE

// namespace DPP *************************************************************
namespace DPP {

// Local-scope objects -------------------------------------------------------
#define LED2_PIN    1
#define LED2_PORT   gioPORTB

#define LED3_PIN    2
#define LED3_PORT   gioPORTB

// NOTE: Switch-A is multiplexed on the same port/pin as LED3,
// so you can use one or the other but not both simultaneously.
//
#define SWA_PIN     2
#define SWA_PORT    gioPORTB

#define SWB_PIN     15
#define SWB_PORT    hetREG1

#define VIM_RAM     ((t_isrFuncPTR *)0xFFF82000U)

static uint32_t l_rnd; // random seed

#ifdef Q_SPY

    // QS source IDs
    static QP::QSpyId const l_rtiCompare0 = { 0U };
    static QP::QSpyId const l_ssiTest = { 0U };

    enum AppRecords { // application-specific trace records
        PHILO_STAT = QP::QS_USER,
        COMMAND_STAT
    };

#endif

} // namespace DPP

// ISRs used in this project =================================================
extern "C" {
//............................................................................
// CAUTION: ISRs MUST be both __stackless and __arm!
QK_IRQ_BEGIN(rtiCompare0)
    // state of the button debouncing, see below
    static struct ButtonsDebouncing {
        uint32_t depressed;
        uint32_t previous;
    } buttons = { ~0U, ~0U };
    uint32_t current;
    uint32_t tmp;

    rtiREG1->INTFLAG = 1U;    // clear the interrutp source
    QP::QTimeEvt::TICK_X(0U, &DPP::l_rtiCompare0); // process time events for rate 0

    // Perform the debouncing of buttons. The algorithm for debouncing
    // adapted from the book "Embedded Systems Dictionary" by Jack Ganssle
    // and Michael Barr, page 71.
    //
    current = ~SWB_PORT->DIN; // read SWB
    tmp = buttons.depressed; // save the debounced depressed buttons
    buttons.depressed |= (buttons.previous & current); // set depressed
    buttons.depressed &= (buttons.previous | current); // clear released
    buttons.previous   = current; // update the history
    tmp ^= buttons.depressed;     // changed debounced depressed
    if ((tmp & (1U << SWB_PIN)) != 0U) {  // debounced SWB state changed?
        if ((buttons.depressed & (1U << SWB_PIN)) != 0U) { // SWB depressed?
            static QP::QEvt const pauseEvt = { DPP::PAUSE_SIG, 0U, 0U};
            QP::QF::PUBLISH(&pauseEvt, &DPP::l_rtiCompare0);
        }
        else {            // the button is released
            static QP::QEvt const serveEvt = { DPP::SERVE_SIG, 0U, 0U};
            QP::QF::PUBLISH(&serveEvt, &DPP::l_rtiCompare0);
        }
    }
QK_IRQ_END()
//............................................................................
QK_IRQ_BEGIN(ssiTest)  // System Software Interrupt for testing
    systemREG1->SSIF = 0x01; // clear the SSI0 source
    // for testing...
    DPP::AO_Table->POST(Q_NEW(QP::QEvt, DPP::MAX_PUB_SIG), &DPP::l_ssiTest);

QK_IRQ_END()
//............................................................................
#ifdef Q_SPY
//
// ISR for receiving bytes from the QSPY Back-End
// NOTE: This ISR is "QP-unaware" meaning that it does not interact with
// the QP and is not disabled. Such ISRs don't need to be defined with
// QK_IRQ_BEGIN()/QK_IRQ_END().
//
#if defined __IAR_SYSTEMS_ICC__
    FIQ
#elif defined __TI_ARM__
    #pragma CODE_STATE(32)
    #pragma INTERRUPT(FIQ)
#else
    #error Unsupported compiler
#endif
void sciHighLevel(void) {
    uint32_t vec = scilinREG->INTVECT0;
    if (vec == 11U) { // SCI receive interrupt
        uint32_t b = scilinREG->RD;
        QP::QS::rxPut(b);
    }
}
#endif // Q_SPY

} // extern "C"

namespace DPP {

// BSP functions =============================================================
void BSP::init(void) {
    // configure the LEDs
    gioInit();
    LED2_PORT->DIR |= (1U << LED2_PIN);  // set as output
    LED3_PORT->DIR |= (1U << LED3_PIN);  // set as output

    // configure the Buttons
    SWB_PORT->DIR  &= (1U << SWB_PIN);    // set as input

    // initialize the random seed
    BSP::randomSeed(1234U);

    if (QS_INIT(nullptr) == 0) { // initialize the QS software tracing
        Q_ERROR();
    }
    QS_OBJ_DICTIONARY(&l_rtiCompare0);
    QS_OBJ_DICTIONARY(&l_ssiTest);
    QS_USR_DICTIONARY(PHILO_STAT);
    QS_USR_DICTIONARY(COMMAND_STAT);

    // setup the QS filters...
    QS_GLB_FILTER(QP::QS_SM_RECORDS); // state machine records
    QS_GLB_FILTER(QP::QS_AO_RECORDS); // active object records
    QS_GLB_FILTER(QP::QS_UA_RECORDS); // all user records
}
//............................................................................
void BSP::displayPhilStat(uint8_t n, char const *stat) {
    if (stat[0] == 'e') {
        LED2_PORT->DSET = (1U << LED2_PIN);
    }
    else {
        LED2_PORT->DCLR = (1U << LED2_PIN);
    }

    QS_BEGIN_ID(PHILO_STAT, AO_Philo[n]->m_prio) // app-specific record begin
        QS_U8(1, n);  // Philosopher number
        QS_STR(stat); // Philosopher status
    QS_END()
}
//............................................................................
void BSP::displayPaused(uint8_t paused) {
    if (paused != 0U) {
        //LED2_PORT->DSET = (1U << LED2_PIN);
    }
    else {
        //LED2_PORT->DCLR = (1U << LED2_PIN);
    }
}
//............................................................................
uint32_t BSP::random(void) { // a very cheap pseudo-random-number generator
    // The flating point code is to exercise the FPU
    float volatile x = 3.1415926F;
    x = x + 2.7182818F;

    // lock the scheduler around l_rnd up to the (N_PHILO + 1U) ceiling
    QP::QSchedStatus lockStat = QP::QK::schedLock(N_PHILO + 1U);
    // "Super-Duper" Linear Congruential Generator (LCG)
    // LCG(2^32, 3*7*11*13*23, 0, seed)
    //
    uint32_t rnd = l_rnd * (3U*7U*11U*13U*23U);
    l_rnd = rnd; // set for the next time
    QP::QK::schedUnlock(lockStat); // unlock sched after accessing l_rnd

    return (rnd >> 8);
}
//............................................................................
void BSP::randomSeed(uint32_t seed) {
    l_rnd = seed;
}

//............................................................................
void BSP::terminate(int16_t result) {
    (void)result;
}

} // namespace DPP


// namespace QP **************************************************************
namespace QP {

// QF callbacks ==============================================================
void QF::onStartup(void) {
    rtiInit(); // configure RTI with UC counter of 7
    rtiSetPeriod(rtiCOUNTER_BLOCK0,
                 (uint32)((RTI_FREQ*1E6/(7+1))/DPP::BSP::TICKS_PER_SEC));
    rtiEnableNotification(rtiNOTIFICATION_COMPARE0);
    rtiStartCounter(rtiCOUNTER_BLOCK0);

    VIM_RAM[2 + 1] = (t_isrFuncPTR)&rtiCompare0; // install the IRQ
    vimREG->FIRQPR0 &= ~(1U << 2);   // designate interrupt as IRQ, NOTE00
    vimREG->REQMASKSET0 = (1U << 2); // enable interrupt

    VIM_RAM[21 + 1] = (t_isrFuncPTR)&ssiTest ; // install the IRQ
    vimREG->FIRQPR0 &= ~(1U << 21);   // designate interrupt as IRQ, NOTE00
    vimREG->REQMASKSET0 = (1U << 21); // enable interrupt

    QF_INT_ENABLE_ALL(); // enable all interrupts (IRQ and FIQ)
}
//............................................................................
void QF::onCleanup(void) {
}
//............................................................................
void QK::onIdle(void) {
    // toggle the User LED on and then off, see NOTE01
    QF_INT_DISABLE();
    LED3_PORT->DSET = (1U << LED3_PIN);
    LED3_PORT->DCLR = (1U << LED3_PIN);
    QF_INT_ENABLE();

#ifdef Q_SPY
    QS::rxParse();  // parse all the received bytes

    //if (sciIsTxReady(scilinREG)) {
    if ((scilinREG->FLR & (uint32)SCI_TX_INT) != 0U) {  // is TX empty?
        uint16_t b;

        QF_INT_DISABLE();
        b = QS::getByte();
        QF_INT_ENABLE();

        if (b != QS_EOD) {  // not End-Of-Data?
            //sciSendByte(scilinREG, (b & 0xFFU));
            scilinREG->TD = (b & 0xFFU);  // put into the TD register
        }
    }
#elif defined NDEBUG
    // Put the CPU and peripherals to the low-power mode.
    // you might need to customize the clock management for your application,
    // see the datasheet for your particular Cortex-R MCU.
    //
    _gotoCPUIdle_(); // wait for interrupt
#endif
}

//............................................................................
extern "C" Q_NORETURN Q_onAssert(char const * const module, int_t const loc) {
    //
    // NOTE: add here your application-specific error handling
    //
    (void)module;
    (void)loc;
    QS_ASSERTION(module, loc, static_cast<uint32_t>(10000U));

#ifndef NDEBUG
    // light up both LEDs
    LED2_PORT->DSET = (1U << LED2_PIN);
    LED3_PORT->DSET = (1U << LED3_PIN);
    // for debugging, hang on in an endless loop until SWB is pressed...
    while ((SWB_PORT->DIN & (1U << SWB_PIN)) != 0) {
    }
#endif

    systemREG1->SYSECR = 0; // perform system reset
}

// QS callbacks ==============================================================
#ifdef Q_SPY
//............................................................................
bool QS::onStartup(void const *arg) {
    static uint8_t qsTxBuf[2*1024]; // buffer for QS transmit channel
    static uint8_t qsRxBuf[100];    // buffer for QS receive channel
    initBuf  (qsTxBuf, sizeof(qsTxBuf));
    rxInitBuf(qsRxBuf, sizeof(qsRxBuf));

    // the SCI (UART) is configured in HALCoGen for 8-n-1 and 115200 baud
    sciInit();

    VIM_RAM[13 + 1] = (t_isrFuncPTR)&sciHighLevel; // install the ISR
    vimREG->FIRQPR0 |= (1U << 13);   // designate interrupt as FIQ
    vimREG->REQMASKSET0 = (1U << 13); // enable interrupt

    return true; // return success
}
//............................................................................
void QS::onCleanup(void) {
}
//............................................................................
QSTimeCtr QS::onGetTime(void) {  // NOTE: invoked with interrupts DISABLED
    return rtiREG1->CNT[0].FRCx; // free running RTI counter0
}
//............................................................................
void QS::onFlush(void) {
    uint16_t b;

    QF_INT_DISABLE();
    while ((b = getByte()) != QS_EOD) { // while not End-Of-Data...
        QF_INT_ENABLE();
        //while (!sciIsTxReady(scilinREG)) {
        while ((scilinREG->FLR & (uint32)SCI_TX_INT) == 0U) {
        }
        //sciSendByte(scilinREG, (b & 0xFFU));
        scilinREG->TD = (b & 0xFFU);
        QF_INT_DISABLE();
    }
    QF_INT_ENABLE();
}
//............................................................................
//! callback function to reset the target (to be implemented in the BSP)
void QS::onReset(void) {
    systemREG1->SYSECR = 0; // perform system reset
}
//............................................................................
//! callback function to execute a user command (to be implemented in BSP)
extern "C" void assert_failed(char const *module, int loc);
void QS::onCommand(uint8_t cmdId, uint32_t param1,
                   uint32_t param2, uint32_t param3)
{
    (void)cmdId;
    (void)param1;
    (void)param2;
    (void)param3;
}

#endif // Q_SPY
//----------------------------------------------------------------------------

} // namespace QP

//============================================================================
// NOTE00:
// The FIQ-type interrupts are never disabled in this QP port, therefore
// they can always preempt any code, including the IRQ-handlers (ISRs).
// Therefore, FIQ-type interrupts are "kernel-unaware" and must NEVER call
// any QP services, such as posting events.
//
// NOTE01:
// One of the LEDs is used to visualize the idle loop activity. The brightness
// of the LED is proportional to the frequency of invcations of the idle loop.
// Please note that the LED is toggled with interrupts locked, so no interrupt
// execution time contributes to the brightness of the User LED.
//
