//****************************************************************************
// Product: DPP example, STM32F4-Discovery board, ThreadX kernel
// Last updated for version 5.8.0
// Last updated on  2016-11-30
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) Quantum Leaps, LLC. All rights reserved.
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
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//
// Contact information:
// http://www.state-machine.com
// mailto:info@state-machine.com
//****************************************************************************
#include "qpcpp.h"
#include "dpp.h"
#include "bsp.h"

#include "stm32f4xx.h"  // CMSIS-compliant header file for the MCU used
#include "stm32f4xx_exti.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_usart.h"
// add other drivers if necessary...

Q_DEFINE_THIS_FILE

// namespace DPP *************************************************************
namespace DPP {

// Local-scope objects -------------------------------------------------------
#define LED_GPIO_PORT     GPIOD
#define LED_GPIO_CLK      RCC_AHB1Periph_GPIOD

#define LED4_PIN          GPIO_Pin_12
#define LED3_PIN          GPIO_Pin_13
#define LED5_PIN          GPIO_Pin_14
#define LED6_PIN          GPIO_Pin_15

#define BTN_GPIO_PORT     GPIOA
#define BTN_GPIO_CLK      RCC_AHB1Periph_GPIOA
#define BTN_B1            GPIO_Pin_0

static unsigned l_rnd; // random seed

#ifdef Q_SPY
    QP::QSTimeCtr QS_tickTime_;
    QP::QSTimeCtr QS_tickPeriod_;

    enum AppRecords { // application-specific trace records
        PHILO_STAT = QP::QS_USER
    };
#endif

extern "C" {

// ISRs used in this project =================================================

} // extern "C"

// BSP functions =============================================================
void BSP::init(void) {
    // NOTE: SystemInit() already called from the startup code
    //  but SystemCoreClock needs to be updated
    //
    SystemCoreClockUpdate();

    // Explictily Disable the automatic FPU state preservation as well as
    // the FPU lazy stacking
    //
    FPU->FPCCR &= ~((1U << FPU_FPCCR_ASPEN_Pos) | (1U << FPU_FPCCR_LSPEN_Pos));

    // Initialize thr port for the LEDs
    RCC_AHB1PeriphClockCmd(LED_GPIO_CLK , ENABLE);

    // GPIO Configuration for the LEDs...
    GPIO_InitTypeDef GPIO_struct;
    GPIO_struct.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_struct.GPIO_OType = GPIO_OType_PP;
    GPIO_struct.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_struct.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_struct.GPIO_Pin = LED3_PIN;
    GPIO_Init(LED_GPIO_PORT, &GPIO_struct);
    LED_GPIO_PORT->BSRRH = LED3_PIN; // turn LED off

    GPIO_struct.GPIO_Pin = LED4_PIN;
    GPIO_Init(LED_GPIO_PORT, &GPIO_struct);
    LED_GPIO_PORT->BSRRH = LED4_PIN; // turn LED off

    GPIO_struct.GPIO_Pin = LED5_PIN;
    GPIO_Init(LED_GPIO_PORT, &GPIO_struct);
    LED_GPIO_PORT->BSRRH = LED5_PIN; // turn LED off

    GPIO_struct.GPIO_Pin = LED6_PIN;
    GPIO_Init(LED_GPIO_PORT, &GPIO_struct);
    LED_GPIO_PORT->BSRRH = LED6_PIN; // turn LED off

    // Initialize thr port for Button
    RCC_AHB1PeriphClockCmd(BTN_GPIO_CLK , ENABLE);

    // GPIO Configuration for the Button...
    GPIO_struct.GPIO_Pin   = BTN_B1;
    GPIO_struct.GPIO_Mode  = GPIO_Mode_IN;
    GPIO_struct.GPIO_OType = GPIO_OType_PP;
    GPIO_struct.GPIO_PuPd  = GPIO_PuPd_DOWN;
    GPIO_struct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(BTN_GPIO_PORT, &GPIO_struct);

    BSP::randomSeed(1234U);

    if (!QS_INIT((void *)0)) { // initialize the QS software tracing
        Q_ERROR();
    }
}
//............................................................................
void BSP::displayPhilStat(uint8_t n, char const *stat) {
    // exercise the FPU with some floating point computations
    float volatile x;
    x = 3.1415926F;
    x = x + 2.7182818F;

    if (stat[0] == 'h') {
        LED_GPIO_PORT->BSRRL = LED3_PIN; // turn LED on
    }
    else {
        LED_GPIO_PORT->BSRRH = LED3_PIN; // turn LED off
    }
    if (stat[0] == 'e') {
        LED_GPIO_PORT->BSRRL = LED5_PIN; // turn LED on
    }
    else {
        LED_GPIO_PORT->BSRRH = LED5_PIN; // turn LED on
    }
    (void)n; // unused parameter (in all but Spy build configuration)

    QS_BEGIN(PHILO_STAT, AO_Philo[n]) // application-specific record begin
        QS_U8(1, n);  // Philosopher number
        QS_STR(stat); // Philosopher status
    QS_END()
}
//............................................................................
void BSP::displayPaused(uint8_t paused) {
    if (paused) {
        LED_GPIO_PORT->BSRRL = LED4_PIN; // turn LED on
    }
    else {
        LED_GPIO_PORT->BSRRH = LED4_PIN; // turn LED on
    }
}
//............................................................................
uint32_t BSP::random(void) { // a very cheap pseudo-random-number generator
    // "Super-Duper" Linear Congruential Generator (LCG)
    // LCG(2^32, 3*7*11*13*23, 0, seed)
    //
    l_rnd = l_rnd * (3U*7U*11U*13U*23U);

    return l_rnd >> 8;
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

static TX_TIMER l_tick_timer; // ThreadX timer to call QF::tickX_()

#ifdef Q_SPY
    // ThreadX thread and thread function for QS output, see NOTE1
    static TX_THREAD l_qs_output_thread;
    static void qs_thread_function(ULONG thread_input);
    static ULONG qs_thread_stkSto[64];
#endif

// QF callbacks ==============================================================
void QF::onStartup(void) {
    //
    // NOTE:
    // This application uses the ThreadX timer to periodically call
    // the QF_tickX_(0) function. Here, only the clock tick rate of 0
    // is used, but other timers can be used to call QF_tickX_() for
    // other clock tick rates, if needed.
    //
    // The choice of a ThreadX timer is not the only option. Applications
    // might choose to call QF_tickX_() directly from timer interrupts
    // or from active object(s).
    //
    Q_ALLEGE(tx_timer_create(&l_tick_timer, // ThreadX timer object
                 "QF",     // name of the timer
                 (VOID (*)(ULONG))&QP::QF::tickX_, // expiration function
                 0U,       // expiration function input (tick rate)
                 1U,       // initial ticks
                 1U,       // reschedule ticks
                 TX_AUTO_ACTIVATE) // automatically activate timer
             == TX_SUCCESS);

#ifdef Q_SPY
    // start a ThreadX timer to perform QS output. See NOTE1...
    Q_ALLEGE(tx_thread_create(&l_qs_output_thread, // thread control block
                 "QS",                  // thread name
                 &qs_thread_function,   // thread function
                 (ULONG)0,              // thread input (unsued)
                 qs_thread_stkSto,      // stack start
                 sizeof(qs_thread_stkSto), // stack size in bytes
                 TX_MAX_PRIORITIES - 1, // ThreadX priority (lowest possible)
                 TX_MAX_PRIORITIES - 1, // preemption threshold disabled
                 TX_NO_TIME_SLICE,
                 TX_AUTO_START)
             == TX_SUCCESS);
#endif // Q_SPY
}
//............................................................................
void QF::onCleanup(void) {
}

//............................................................................
extern "C" void Q_onAssert(char const *module, int loc) {
    //
    // NOTE: add here your application-specific error handling
    //
    (void)module;
    (void)loc;
    QS_ASSERTION(module, loc, static_cast<uint32_t>(10000U));
    NVIC_SystemReset();
}

// QS callbacks ==============================================================
#ifdef Q_SPY

//............................................................................
static void qs_thread_function(ULONG /*thread_input*/) { // see NOTE1
    for (;;) {
        // turn the LED6 on an off to visualize the QS activity
        LED_GPIO_PORT->BSRRL = LED6_PIN; // turn LED on
        __NOP(); // wait a little to actually see the LED glow
        __NOP();
        __NOP();
        __NOP();
        LED_GPIO_PORT->BSRRH = LED6_PIN; // turn LED off

        if ((USART2->SR & 0x80U) != 0U) {  // is TXE empty?
            uint16_t b;
            QF_CRIT_STAT_TYPE intStat;

            QF_CRIT_ENTRY(intStat);
            b = QP::QS::getByte();
            QF_CRIT_EXIT(intStat);

            if (b != QP::QS_EOD) {  // not End-Of-Data?
                USART2->DR  = (b & 0xFFU);  // put into the DR register
            }
        }
        // no blocking in this thread; see NOTE1
    }
}

//............................................................................
bool QS::onStartup(void const *arg) {
    static uint8_t qsBuf[1024]; // buffer for Quantum Spy
    initBuf(qsBuf, sizeof(qsBuf));

    GPIO_InitTypeDef GPIO_struct;
    USART_InitTypeDef USART_struct;

    // enable peripheral clock for USART2
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    // GPIOA clock enable
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    // GPIOA Configuration:  USART2 TX on PA2
    GPIO_struct.GPIO_Pin = GPIO_Pin_2;
    GPIO_struct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_struct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_struct.GPIO_OType = GPIO_OType_PP;
    GPIO_struct.GPIO_PuPd = GPIO_PuPd_UP ;
    GPIO_Init(GPIOA, &GPIO_struct);

    // Connect USART2 pins to AF2
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2); // TX = PA2
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2); // RX = PA3

    USART_struct.USART_BaudRate = 115200;
    USART_struct.USART_WordLength = USART_WordLength_8b;
    USART_struct.USART_StopBits = USART_StopBits_1;
    USART_struct.USART_Parity = USART_Parity_No;
    USART_struct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_struct.USART_Mode = USART_Mode_Tx;
    USART_Init(USART2, &USART_struct);

    USART_Cmd(USART2, ENABLE); // enable USART2

    // setup the QS filters...
    QS_FILTER_ON(QS_QEP_STATE_ENTRY);
    QS_FILTER_ON(QS_QEP_STATE_EXIT);
    QS_FILTER_ON(QS_QEP_STATE_INIT);
    QS_FILTER_ON(QS_QEP_INIT_TRAN);
    QS_FILTER_ON(QS_QEP_INTERN_TRAN);
    QS_FILTER_ON(QS_QEP_TRAN);
    QS_FILTER_ON(QS_QEP_IGNORED);
    QS_FILTER_ON(QS_QEP_DISPATCH);
    QS_FILTER_ON(QS_QEP_UNHANDLED);

    QS_FILTER_ON(DPP::PHILO_STAT);

    return true; // return success
}
//............................................................................
void QS::onCleanup(void) {
}
//............................................................................
QSTimeCtr QS::onGetTime(void) {  // NOTE: invoked with interrupts DISABLED
    if ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0U) { // not set?
        return DPP::QS_tickTime_ - static_cast<QSTimeCtr>(SysTick->VAL);
    }
    else { // the rollover occured, but the SysTick_ISR did not run yet
        return DPP::QS_tickTime_ + DPP::QS_tickPeriod_
               - static_cast<QSTimeCtr>(SysTick->VAL);
    }
}
//............................................................................
void QS::onFlush(void) {
    uint16_t b;
    QF_CRIT_STAT_TYPE intStat;

    QF_CRIT_ENTRY(intStat);
    while ((b = getByte()) != QS_EOD) {  // while not End-Of-Data...
        QF_CRIT_EXIT(intStat);
        while ((USART2->SR & USART_FLAG_TXE) == 0U) { // while TXE not empty
        }
        USART2->DR = (b & 0xFFU); // put into the DR register
        QF_CRIT_ENTRY(intStat);
    }
    QF_CRIT_EXIT(intStat);
}
//............................................................................
//! callback function to reset the target (to be implemented in the BSP)
void QS::onReset(void) {
    //TBD
}
//............................................................................
//! callback function to execute a uesr command (to be implemented in BSP)
void QS::onCommand(uint8_t cmdId, uint32_t param) {
    (void)cmdId;
    (void)param;
    //TBD
}
#endif // Q_SPY
//----------------------------------------------------------------------------

} // namespace QP

//****************************************************************************
// NOTE1:
// This application uses the ThreadX thread of the lowest priority to perform
// the QS data output to the host. This is not the only choice available, and
// other applications might choose to peform the QS output some other way.
//
// The lowest-priority thread does not block, so in effect, it becomes the
// idle loop. This presents no problems to ThreadX - its idle task in the
// scheduler does not need to run.
//

