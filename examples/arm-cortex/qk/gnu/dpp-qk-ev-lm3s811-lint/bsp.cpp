//////////////////////////////////////////////////////////////////////////////
// Product: "Dining Philosophers Problem" example, preemptive QK kernel
// Last Updated for Version: 4.5.02
// Date of the Last Update:  Aug 09, 2012
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2002-2012 Quantum Leaps, LLC. All rights reserved.
//
// This program is open source software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation, either version 2 of the License, or
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
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//
// Contact information:
// Quantum Leaps Web sites: http://www.quantum-leaps.com
//                          http://www.state-machine.com
// e-mail:                  info@quantum-leaps.com
//////////////////////////////////////////////////////////////////////////////
#include "qp_port.h"
#include "dpp.h"
#include "bsp.h"

#include "lm3s_cmsis.h"
#include "display96x16x1.h"

//////////////////////////////////////////////////////////////////////////////
namespace DPP {

Q_DEFINE_THIS_FILE

enum ISR_Priorities {      // ISR priorities starting from the highest urgency
    GPIOPORTA_PRIO,
    SYSTICK_PRIO
    // ...
};

// Local-scope objects -------------------------------------------------------
static uint32_t l_rnd;                                          // random seed

uint32_t const PUSH_BUTTON = static_cast<uint32_t>(1) << 4;
uint32_t const USER_LED    = static_cast<uint32_t>(1) << 5;

#ifdef Q_SPY

    QP::QSTimeCtr QS_tickTime_;
    QP::QSTimeCtr QS_tickPeriod_;
    static uint8_t l_SysTick_Handler;
    static uint8_t l_GPIOPortA_IRQHandler;

    uint32_t const UART_BAUD_RATE    = static_cast<uint32_t>(115200U);
    uint32_t const UART_FR_TXFE      = static_cast<uint32_t>(0x00000080U);
    uint16_t const UART_TXFIFO_DEPTH = static_cast<uint16_t>(16U);

    enum AppRecords {                    // application-specific trace records
        PHILO_STAT = QP::QS_USER
    };

#endif

//............................................................................
extern "C" void SysTick_Handler(void) {
    QK_ISR_ENTRY();                         // inform QK about entering an ISR

#ifdef Q_SPY
    uint32_t dummy = SysTick->CTRL;            // clear SysTick_CTRL_COUNTFLAG
    QS_tickTime_ += QS_tickPeriod_;          // account for the clock rollover
#endif

    QP::QF::TICK(&l_SysTick_Handler);         // process all armed time events

    static uint32_t btn_debounced  = PUSH_BUTTON;
    static uint8_t  debounce_state = 0U;
    uint32_t btn = GPIOC->DATA_Bits[PUSH_BUTTON];         // read the push btn
    switch (debounce_state) {
        case 0:
            if (btn != btn_debounced) {
                debounce_state = 1U;           // transition to the next state
            }
            break;
        case 1:
            if (btn != btn_debounced) {
                debounce_state = 2U;           // transition to the next state
            }
            else {
                debounce_state = 0U;             // transition back to state 0
            }
            break;
        case 2:
            if (btn != btn_debounced) {
                debounce_state = 3U;           // transition to the next state
            }
            else {
                debounce_state = 0U;             // transition back to state 0
            }
            break;
        case 3:
            if (btn != btn_debounced) {
                btn_debounced = btn;        // save the debounced button value

                if (btn == 0U) {                   // is the button depressed?
                    static QP::QEvt const pauseEvt =
                        QEVT_INITIALIZER(PAUSE_SIG);
                    QP::QF::PUBLISH(&pauseEvt, &l_SysTick_Handler);
                }
                else {
                    static QP::QEvt const pauseEvt =
                        QEVT_INITIALIZER(PAUSE_SIG);
                    QP::QF::PUBLISH(&pauseEvt, &l_SysTick_Handler);
                }
            }
            debounce_state = 0U;                 // transition back to state 0
            break;
        default:
            Q_ERROR();
            break;
    }
    QK_ISR_EXIT();                           // inform QK about exiting an ISR
}
//.............................................................................
extern "C" void GPIOPortA_IRQHandler(void) {
    QK_ISR_ENTRY();                         // infrom QK about entering an ISR

    DPP::AO_Table->POST(Q_NEW(QP::QEvt, DPP::MAX_PUB_SIG),    // for testing
                   &l_GPIOPortA_IRQHandler);

    QK_ISR_EXIT();                           // infrom QK about exiting an ISR
}

//............................................................................
void BSP_init(void) {
    // set the system clock as specified in lm3s_config.h (20MHz from PLL)
    SystemInit();

    // enable clock to the peripherals used by the application
    SYSCTL->RCGC2 |= 1U | (1U << 2);              // enable clock to GPIOA & C
    __NOP();                                     // wait after enabling clocks
    __NOP();
    __NOP();

    // configure the LED and push button
    GPIOC->DIR |= USER_LED;                           // set direction: output
    GPIOC->DEN |= USER_LED;                                  // digital enable
    GPIOC->DATA_Bits[USER_LED] = 0U;                  // turn the User LED off

    GPIOC->DIR &= ~PUSH_BUTTON;                       //  set direction: input
    GPIOC->DEN |= PUSH_BUTTON;                               // digital enable

    Display96x16x1Init(1U);                     // initialize the OLED display
    Display96x16x1StringDraw(&"Dining Philos"[0], 0U, 0U);
    Display96x16x1StringDraw(&"0 ,1 ,2 ,3 ,4"[0], 0U, 1U);

    Q_ALLEGE(QS_INIT(static_cast<void *>(0)));
    QS_RESET();
    QS_OBJ_DICTIONARY(&l_SysTick_Handler);
    QS_OBJ_DICTIONARY(&l_GPIOPortA_IRQHandler);
    QS_USR_DICTIONARY(PHILO_STAT);
}
//............................................................................
void BSP_displayPhilStat(uint8_t const n, char_t const * const stat) {
    char_t str[2];
    str[0] = *stat;
    str[1] = '\0';
    Display96x16x1StringDraw(&str[0],
                             (3U*6U*static_cast<uint32_t>(n)) + 6U, 1U);

    QS_BEGIN(PHILO_STAT, AO_Philo[n])     // application-specific record begin
        QS_U8(1U, n);                                    // Philosopher number
        QS_STR(stat);                                    // Philosopher status
    QS_END()
}
//............................................................................
void BSP_displayPaused(uint8_t const paused) {
    Display96x16x1StringDraw((paused != 0U) ? "P" : " ",
                             static_cast<uint32_t>(15U*6U), 0U);
}
//............................................................................
uint32_t BSP_random(void) {     // a very cheap pseudo-random-number generator
    // "Super-Duper" Linear Congruential Generator (LCG)
    // LCG(2^32, 3*7*11*13*23, 0, seed)
    //
    l_rnd = l_rnd * (3U*7U*11U*13U*23U);
    return l_rnd >> 8;
}
//............................................................................
void BSP_randomSeed(uint32_t const seed) {
    l_rnd = seed;
}
//............................................................................
void BSP_terminate(int16_t const result) {
    (void)result;
}

}                                                             // namespace DPP
//////////////////////////////////////////////////////////////////////////////

//............................................................................
extern "C" void Q_onAssert(char_t const Q_ROM * const Q_ROM_VAR file, int_t const line) {
    (void)file;                                      // avoid compiler warning
    (void)line;                                      // avoid compiler warning
    QF_INT_DISABLE();            // make sure that all interrupts are disabled
    for (;;) {          // NOTE: replace the loop with reset for final version
    }
}
//............................................................................
// error routine that is called if the CMSIS library encounters an error
extern "C" void assert_failed(char const *file, int line) {
    Q_onAssert(file, line);
}


namespace QP {

//............................................................................
void QF::onStartup(void) {
                 // set up the SysTick timer to fire at BSP_TICKS_PER_SEC rate
    (void)SysTick_Config(SystemFrequency / DPP::BSP_TICKS_PER_SEC);

                          // set priorities of all interrupts in the system...
    NVIC_SetPriority(SysTick_IRQn,   DPP::SYSTICK_PRIO);
    NVIC_SetPriority(GPIOPortA_IRQn, DPP::GPIOPORTA_PRIO);

    NVIC_EnableIRQ(GPIOPortA_IRQn);
}
//............................................................................
void QF::onCleanup(void) {
}
//............................................................................
void QK::onIdle(void) {

    // toggle the User LED on and then off, see NOTE01
    QF_INT_DISABLE();
    GPIOC->DATA_Bits[DPP::USER_LED] = DPP::USER_LED;   // turn the User LED on
    GPIOC->DATA_Bits[DPP::USER_LED] = 0U;             // turn the User LED off
    QF_INT_ENABLE();

#ifdef Q_SPY
    if ((UART0->FR & DPP::UART_FR_TXFE) != 0U) {                   // TX done?
        uint16_t fifo = DPP::UART_TXFIFO_DEPTH;     // max bytes we can accept

        QF_INT_DISABLE();
        uint8_t const *block = QS::getBlock(&fifo);   // try to get next block
        QF_INT_ENABLE();

        while (fifo-- != 0) {                       // any bytes in the block?
            UART0->DR = *block++;                         // put into the FIFO
        }
    }
#elif defined NDEBUG
    // put the CPU and peripherals to the low-power mode
    // you might need to customize the clock management for your application,
    // see the datasheet for your particular Cortex-M3 MCU.
    __WFI();                                             // Wait-For-Interrupt
#endif
}

//----------------------------------------------------------------------------
#ifdef Q_SPY
//............................................................................
bool QS::onStartup(void const *) {
    static uint8_t qsBuf[6*256];                     // buffer for Quantum Spy
    uint32_t tmp;
    initBuf(qsBuf, sizeof(qsBuf));

                                   // enable the peripherals used by the UART0
    SYSCTL->RCGC1 |= (1U << 0);                       // enable clock to UART0
    SYSCTL->RCGC2 |= (1U << 0);                       // enable clock to GPIOA
    __NOP();                                     // wait after enabling clocks
    __NOP();
    __NOP();

                                    // configure UART0 pins for UART operation
    tmp = (1 << 0) | (1 << 1);
    GPIOA->DIR   &= ~tmp;
    GPIOA->AFSEL |= tmp;
    GPIOA->DR2R  |= tmp;           // set 2mA drive, DR4R and DR8R are cleared
    GPIOA->SLR   &= ~tmp;
    GPIOA->ODR   &= ~tmp;
    GPIOA->PUR   &= ~tmp;
    GPIOA->PDR   &= ~tmp;
    GPIOA->DEN   |= tmp;

              // configure the UART for the desired baud rate, 8-N-1 operation
    tmp = (((SystemFrequency * 8U) / DPP::UART_BAUD_RATE) + 1U) / 2U;
    UART0->IBRD   = tmp / 64U;
    UART0->FBRD   = tmp % 64U;
    UART0->LCRH   = 0x60U;                         // configure 8-N-1 operation
    UART0->LCRH  |= 0x10U;
    UART0->CTL   |= (1U << 0) | (1U << 8) | (1U << 9);

    DPP::QS_tickPeriod_ = SystemFrequency / DPP::BSP_TICKS_PER_SEC;
    DPP::QS_tickTime_ = DPP::QS_tickPeriod_; // to start the timestamp at zero

                                                    // setup the QS filters...
    QS_FILTER_ON(QS_ALL_RECORDS);

//    QS_FILTER_OFF(QS_QEP_STATE_EMPTY);
//    QS_FILTER_OFF(QS_QEP_STATE_ENTRY);
//    QS_FILTER_OFF(QS_QEP_STATE_EXIT);
//    QS_FILTER_OFF(QS_QEP_STATE_INIT);
//    QS_FILTER_OFF(QS_QEP_INIT_TRAN);
//    QS_FILTER_OFF(QS_QEP_INTERN_TRAN);
//    QS_FILTER_OFF(QS_QEP_TRAN);
//    QS_FILTER_OFF(QS_QEP_IGNORED);

//    QS_FILTER_OFF(QS_QF_ACTIVE_ADD);
//    QS_FILTER_OFF(QS_QF_ACTIVE_REMOVE);
//    QS_FILTER_OFF(QS_QF_ACTIVE_SUBSCRIBE);
//    QS_FILTER_OFF(QS_QF_ACTIVE_UNSUBSCRIBE);
//    QS_FILTER_OFF(QS_QF_ACTIVE_POST_FIFO);
//    QS_FILTER_OFF(QS_QF_ACTIVE_POST_LIFO);
//    QS_FILTER_OFF(QS_QF_ACTIVE_GET);
//    QS_FILTER_OFF(QS_QF_ACTIVE_GET_LAST);
//    QS_FILTER_OFF(QS_QF_EQUEUE_INIT);
//    QS_FILTER_OFF(QS_QF_EQUEUE_POST_FIFO);
//    QS_FILTER_OFF(QS_QF_EQUEUE_POST_LIFO);
//    QS_FILTER_OFF(QS_QF_EQUEUE_GET);
//    QS_FILTER_OFF(QS_QF_EQUEUE_GET_LAST);
//    QS_FILTER_OFF(QS_QF_MPOOL_INIT);
//    QS_FILTER_OFF(QS_QF_MPOOL_GET);
//    QS_FILTER_OFF(QS_QF_MPOOL_PUT);
//    QS_FILTER_OFF(QS_QF_PUBLISH);
//    QS_FILTER_OFF(QS_QF_NEW);
//    QS_FILTER_OFF(QS_QF_GC_ATTEMPT);
//    QS_FILTER_OFF(QS_QF_GC);
//    QS_FILTER_OFF(QS_QF_TICK);
//    QS_FILTER_OFF(QS_QF_TIMEEVT_ARM);
//    QS_FILTER_OFF(QS_QF_TIMEEVT_AUTO_DISARM);
//    QS_FILTER_OFF(QS_QF_TIMEEVT_DISARM_ATTEMPT);
//    QS_FILTER_OFF(QS_QF_TIMEEVT_DISARM);
//    QS_FILTER_OFF(QS_QF_TIMEEVT_REARM);
//    QS_FILTER_OFF(QS_QF_TIMEEVT_POST);
    QS_FILTER_OFF(QS_QF_CRIT_ENTRY);
    QS_FILTER_OFF(QS_QF_CRIT_EXIT);
    QS_FILTER_OFF(QS_QF_ISR_ENTRY);
    QS_FILTER_OFF(QS_QF_ISR_EXIT);

    return true;                                             // return success
}
//............................................................................
void QS::onCleanup(void) {
}
//............................................................................
QSTimeCtr QS::onGetTime(void) {            // invoked with interrupts disabled
    QSTimeCtr ret = DPP::QS_tickTime_ - static_cast<QSTimeCtr>(SysTick->VAL);
    if ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) != 0U) {     // flag set?
        ret += DPP::QS_tickPeriod_;
    }
    return ret;
}
//............................................................................
void QS::onFlush(void) {
    uint16_t fifo = DPP::UART_TXFIFO_DEPTH;                   // Tx FIFO depth
    uint8_t const *block;
    while ((block = getBlock(&fifo)) != static_cast<uint8_t *>(0)) {
                                              // busy-wait until TX FIFO empty
        while ((UART0->FR & DPP::UART_FR_TXFE) == 0U) {
        }

        while (fifo-- != 0U) {                      // any bytes in the block?
            UART0->DR = *block++;                      // put into the TX FIFO
        }
        fifo = DPP::UART_TXFIFO_DEPTH;            // re-load the Tx FIFO depth
    }
}
#endif                                                                // Q_SPY
//----------------------------------------------------------------------------

}                                                              // namespace QP

//////////////////////////////////////////////////////////////////////////////
// NOTE01:
// The User LED is used to visualize the idle loop activity. The brightness
// of the LED is proportional to the frequency of invcations of the idle loop.
// Please note that the LED is toggled with interrupts locked, so no interrupt
// execution time contributes to the brightness of the User LED.
//
