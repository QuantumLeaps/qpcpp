//============================================================================
// Product: DPP example, NUCLEO-C031C6 board, QXK kernel, MPU isolation
// Last updated for version 7.3.2
// Last updated on  2023-12-13
//
//                   Q u a n t u m  L e a P s
//                   ------------------------
//                   Modern Embedded Software
//
// Copyright (C) 2005 Quantum Leaps, LLC. <state-machine.com>
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
#include "qpcpp.hpp"             // QP/C++ real-time embedded framework
#include "dpp.hpp"               // DPP Application interface
#include "bsp.hpp"               // Board Support Package

#include "stm32c0xx.h"  // CMSIS-compliant header file for the MCU used
// add other drivers if necessary...

//============================================================================
namespace { // unnamed namespace for local stuff with internal linkage

Q_DEFINE_THIS_FILE

// Local-scope objects -------------------------------------------------------
constexpr std::uint32_t LD4_PIN     {5U};
constexpr std::uint32_t B1_PIN      {13U};

#ifdef Q_SPY

    // QSpy source IDs
    QP::QSTimeCtr QS_tickTime_;
    QP::QSTimeCtr QS_tickPeriod_;

    // QSpy source IDs
    static QP::QSpyId const l_SysTick_Handler = { 0U };
    static QP::QSpyId const l_EXTI0_1_IRQHandler = { 0U };

    enum AppRecords { // application-specific trace records
        PHILO_STAT = QP::QS_USER,
        PAUSED_STAT,
    };

#endif

//----------------------------------------------------------------------------
// MPU storage and settings...
struct MPU_Region {
    std::uint32_t RBAR;
    std::uint32_t RASR;
};

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
    QS_ASSERTION(module, id, 10000U);

#ifndef NDEBUG
    // light up the user LED
    GPIOA->BSRR = (1U << LD4_PIN);  // turn LED on
    // for debugging, hang on in an endless loop...
    for (;;) {
    }
#else
    NVIC_SystemReset();
    for (;;) { // explicitly "no-return"
    }
#endif
}
//............................................................................
void assert_failed(char const * const module, int_t const id); // prototype
void assert_failed(char const * const module, int_t const id) {
    Q_onError(module, id);
}

// ISRs used in the application ==============================================

void SysTick_Handler(void); // prototype
void SysTick_Handler(void) {
    QXK_ISR_ENTRY();   // inform QXK about entering an ISR

    QP::QTimeEvt::TICK_X(0U, &l_SysTick_Handler); // time events at rate 0

    // Perform the debouncing of buttons. The algorithm for debouncing
    // adapted from the book "Embedded Systems Dictionary" by Jack Ganssle
    // and Michael Barr, page 71.
    static struct {
        std::uint32_t depressed;
        std::uint32_t previous;
    } buttons = { 0U, 0U };

    QF_INT_DISABLE();
    QF_MEM_SYS();

    std::uint32_t current = ~GPIOC->IDR; // read Port C with Button B1
    std::uint32_t tmp = buttons.depressed; // save the depressed buttons
    buttons.depressed |= (buttons.previous & current); // set depressed
    buttons.depressed &= (buttons.previous | current); // clear released
    buttons.previous   = current; // update the history
    tmp ^= buttons.depressed;     // changed debounced depressed
    current = buttons.depressed;

#ifdef Q_SPY
    tmp = SysTick->CTRL; // clear CTRL_COUNTFLAG
    QS_tickTime_ += QS_tickPeriod_; // account for the clock rollover
#endif

    QF_MEM_APP();
    QF_INT_ENABLE();

    if ((tmp & (1U << B1_PIN)) != 0U) { // debounced B1 state changed?
        if ((current & (1U << B1_PIN)) != 0U) { // is B1 depressed?
            static QP::QEvt const pauseEvt(APP::PAUSE_SIG);
            QP::QActive::PUBLISH(&pauseEvt, &l_SysTick_Handler);
        }
        else { // the button is released
            static QP::QEvt const serveEvt(APP::SERVE_SIG);
            QP::QActive::PUBLISH(&serveEvt, &l_SysTick_Handler);
        }
    }

    QXK_ISR_EXIT();  // inform QXK about exiting an ISR
}
//............................................................................
// interrupt handler for testing preemptions
void EXTI0_1_IRQHandler(void); // prototype
void EXTI0_1_IRQHandler(void) {
    QXK_ISR_ENTRY();   // inform QXK about entering an ISR

    // for testing..
    static QP::QEvt const testEvt(APP::TEST_SIG);
    APP::AO_Table->POST(&testEvt, &l_EXTI0_1_IRQHandler);

    QXK_ISR_EXIT();  // inform QXK about exiting an ISR
}

//............................................................................
#ifdef Q_SPY
// ISR for receiving bytes from the QSPY Back-End
// NOTE: This ISR is "QF-unaware" meaning that it does not interact with
// the QF/QXK and is not disabled. Such ISRs don't need to call
// QXK_ISR_ENTRY/QXK_ISR_EXIT and they cannot post or publish events.

void USART2_IRQHandler(void); // prototype
void USART2_IRQHandler(void) { // used in QS-RX (kernel UNAWARE interrutp)
    // is RX register NOT empty?
    QF_MEM_SYS();
    if ((USART2->ISR & (1U << 5U)) != 0U) {
        std::uint8_t b = USART2->RDR;
        QP::QS::rxPut(b);
    }

    QXK_ARM_ERRATUM_838869();
}
#endif // Q_SPY

#ifdef QF_MEM_ISOLATE
//............................................................................
__attribute__(( used ))
void QF_onMemSys(void) {
    MPU->CTRL = MPU_CTRL_ENABLE_Msk        // enable the MPU
                | MPU_CTRL_PRIVDEFENA_Msk; // enable background region
    __ISB();
    __DSB();
}
//............................................................................
__attribute__(( used ))
void QF_onMemApp() {
    MPU->CTRL = MPU_CTRL_ENABLE_Msk; // enable the MPU
                // but do NOT enable background region
    __ISB();
    __DSB();
}
//............................................................................
#ifdef QF_ON_CONTEXT_SW
// NOTE: the context-switch callback is called with interrupts DISABLED
void QF_onContextSw(QP::QActive *prev, QP::QActive *next) {
    if (next != nullptr) {
        MPU->CTRL = 0U; // disable the MPU

        MPU_Region const * const region =
            static_cast<MPU_Region const *>(next->getThread());
        MPU->RBAR = region[0].RBAR;
        MPU->RASR = region[0].RASR;
        MPU->RBAR = region[1].RBAR;
        MPU->RASR = region[1].RASR;
        MPU->RBAR = region[2].RBAR;
        MPU->RASR = region[2].RASR;

        MPU->CTRL = MPU_CTRL_ENABLE_Msk        // enable the MPU
                    | MPU_CTRL_PRIVDEFENA_Msk; // enable background region
        __ISB();
        __DSB();
    }
}
#endif // QF_ON_CONTEXT_SW
#endif // QF_MEM_ISOLATE

} // extern "C"

//============================================================================
namespace { // unnamed namespace for local stuff with internal linkage

// Stack .....................................................................
// NOTE
// The main stack size (provided here as power of 2), MUST match the actual
// stack size defined in the linker-script/startup-code.
constexpr std::uint32_t STACK_SIZE_POW2 {11U};

// Table AO...................................................................
// size of Table instance, as power-of-2
constexpr std::uint32_t TABLE_SIZE_POW2 {7U};

__attribute__((aligned((1U << TABLE_SIZE_POW2))))
static std::uint8_t Table_sto[1U << TABLE_SIZE_POW2];

#ifdef QF_MEM_ISOLATE
static MPU_Region const MPU_Table[3] = {
    { reinterpret_cast<std::uint32_t>(Table_sto) + 0x10U,//---- region #0
      ((TABLE_SIZE_POW2 - 1U) << MPU_RASR_SIZE_Pos)   // size
       + (3U << MPU_RASR_AP_Pos)                      // PA:rw/UA:rw
       + (1U << MPU_RASR_XN_Pos)                      // XN=1
       + (1U << MPU_RASR_S_Pos)                       // S=1
       + (1U << MPU_RASR_C_Pos)                       // C=1
       + (0U << MPU_RASR_B_Pos)                       // B=0
       + (0U << MPU_RASR_TEX_Pos)                     // TEX=0
       + MPU_RASR_ENABLE_Msk },                       // region enable
    { GPIOA_BASE + 0x11U,                      //---- region #1
      ((9U - 1U) << MPU_RASR_SIZE_Pos)                // 2^9=512B size
       + (3U << MPU_RASR_AP_Pos)                      // PA:rw/UA:rw
       + (1U << MPU_RASR_XN_Pos)                      // XN=1
       + (1U << MPU_RASR_S_Pos)                       // S=1
       + (0U << MPU_RASR_C_Pos)                       // C=0
       + (1U << MPU_RASR_B_Pos)                       // B=1
       + (0U << MPU_RASR_TEX_Pos)                     // TEX=0
       + MPU_RASR_ENABLE_Msk },                       // region enable
    { 0U + 0x12U,                              //---- region #2
      0U },
};
#endif

// Philo AOs..................................................................
// size of Philo instance, as power-of-2
constexpr std::uint32_t PHILO_SIZE_POW2 {7U};

__attribute__((aligned((1U << PHILO_SIZE_POW2))))
static std::uint8_t Philo_sto[APP::N_PHILO][1U << PHILO_SIZE_POW2];

constexpr std::uint32_t PHILO_SHARED_SIZE_POW2 {5U};
__attribute__((aligned((1U << PHILO_SHARED_SIZE_POW2))))
static union {
    std::uint32_t rnd_seed;
    std::uint8_t byteso[1U << PHILO_SHARED_SIZE_POW2];
} Philo_shared_sto;

#ifdef QF_MEM_ISOLATE
static MPU_Region const MPU_Philo[APP::N_PHILO][3] = {
    {{ reinterpret_cast<std::uint32_t>(Philo_sto[0]) + 0x10U,//---- region #0
       ((PHILO_SIZE_POW2 - 1U) << MPU_RASR_SIZE_Pos)  // size
       + (3U << MPU_RASR_AP_Pos)                      // PA:rw/UA:rw
       + (1U << MPU_RASR_XN_Pos)                      // XN=1
       + (1U << MPU_RASR_S_Pos)                       // S=1
       + (1U << MPU_RASR_C_Pos)                       // C=1
       + (0U << MPU_RASR_B_Pos)                       // B=0
       + (0U << MPU_RASR_TEX_Pos)                     // TEX=0
       + MPU_RASR_ENABLE_Msk },                       // region enable
    { reinterpret_cast<std::uint32_t>(&Philo_shared_sto) + 0x11U,//---- region #1
      ((PHILO_SHARED_SIZE_POW2 - 1U) << MPU_RASR_SIZE_Pos)   // size
       + (3U << MPU_RASR_AP_Pos)                      // PA:rw/UA:rw
       + (1U << MPU_RASR_XN_Pos)                      // XN=1
       + (1U << MPU_RASR_S_Pos)                       // S=1
       + (1U << MPU_RASR_C_Pos)                       // C=1
       + (0U << MPU_RASR_B_Pos)                       // B=0
       + (0U << MPU_RASR_TEX_Pos)                     // TEX=0
       + MPU_RASR_ENABLE_Msk },                       // region enable
    { 0U + 0x12U,                              //---- region #2
      0U }},
    {{ reinterpret_cast<std::uint32_t>(Philo_sto[1]) + 0x10U,//---- region #0
       ((PHILO_SIZE_POW2 - 1U) << MPU_RASR_SIZE_Pos)  // size
       + (3U << MPU_RASR_AP_Pos)                      // PA:rw/UA:rw
       + (1U << MPU_RASR_XN_Pos)                      // XN=1
       + (1U << MPU_RASR_S_Pos)                       // S=1
       + (1U << MPU_RASR_C_Pos)                       // C=1
       + (0U << MPU_RASR_B_Pos)                       // B=0
       + (0U << MPU_RASR_TEX_Pos)                     // TEX=0
       + MPU_RASR_ENABLE_Msk },                       // region enable
    { reinterpret_cast<std::uint32_t>(&Philo_shared_sto) + 0x11U,//---- region #1
      ((PHILO_SHARED_SIZE_POW2 - 1U) << MPU_RASR_SIZE_Pos)   // size
       + (3U << MPU_RASR_AP_Pos)                      // PA:rw/UA:rw
       + (1U << MPU_RASR_XN_Pos)                      // XN=1
       + (1U << MPU_RASR_S_Pos)                       // S=1
       + (1U << MPU_RASR_C_Pos)                       // C=1
       + (0U << MPU_RASR_B_Pos)                       // B=0
       + (0U << MPU_RASR_TEX_Pos)                     // TEX=0
       + MPU_RASR_ENABLE_Msk },                       // region enable
    { 0U + 0x12U,                              //---- region #2
      0U }},
    {{ reinterpret_cast<std::uint32_t>(Philo_sto[2]) + 0x10U,//---- region #0
       ((PHILO_SIZE_POW2 - 1U) << MPU_RASR_SIZE_Pos)  // size
       + (3U << MPU_RASR_AP_Pos)                      // PA:rw/UA:rw
       + (1U << MPU_RASR_XN_Pos)                      // XN=1
       + (1U << MPU_RASR_S_Pos)                       // S=1
       + (1U << MPU_RASR_C_Pos)                       // C=1
       + (0U << MPU_RASR_B_Pos)                       // B=0
       + (0U << MPU_RASR_TEX_Pos)                     // TEX=0
       + MPU_RASR_ENABLE_Msk },                       // region enable
    { reinterpret_cast<std::uint32_t>(&Philo_shared_sto) + 0x11U,//---- region #1
      ((PHILO_SHARED_SIZE_POW2 - 1U) << MPU_RASR_SIZE_Pos)   // size
       + (3U << MPU_RASR_AP_Pos)                      // PA:rw/UA:rw
       + (1U << MPU_RASR_XN_Pos)                      // XN=1
       + (1U << MPU_RASR_S_Pos)                       // S=1
       + (1U << MPU_RASR_C_Pos)                       // C=1
       + (0U << MPU_RASR_B_Pos)                       // B=0
       + (0U << MPU_RASR_TEX_Pos)                     // TEX=0
       + MPU_RASR_ENABLE_Msk },                       // region enable
    { 0U + 0x12U,                              //---- region #2
      0U }},
    {{ reinterpret_cast<std::uint32_t>(Philo_sto[3]) + 0x10U,//---- region #0
       ((PHILO_SIZE_POW2 - 1U) << MPU_RASR_SIZE_Pos)  // size
       + (3U << MPU_RASR_AP_Pos)                      // PA:rw/UA:rw
       + (1U << MPU_RASR_XN_Pos)                      // XN=1
       + (1U << MPU_RASR_S_Pos)                       // S=1
       + (1U << MPU_RASR_C_Pos)                       // C=1
       + (0U << MPU_RASR_B_Pos)                       // B=0
       + (0U << MPU_RASR_TEX_Pos)                     // TEX=0
       + MPU_RASR_ENABLE_Msk },                       // region enable
    { reinterpret_cast<std::uint32_t>(&Philo_shared_sto) + 0x11U,//---- region #1
      ((PHILO_SHARED_SIZE_POW2 - 1U) << MPU_RASR_SIZE_Pos)   // size
       + (3U << MPU_RASR_AP_Pos)                      // PA:rw/UA:rw
       + (1U << MPU_RASR_XN_Pos)                      // XN=1
       + (1U << MPU_RASR_S_Pos)                       // S=1
       + (1U << MPU_RASR_C_Pos)                       // C=1
       + (0U << MPU_RASR_B_Pos)                       // B=0
       + (0U << MPU_RASR_TEX_Pos)                     // TEX=0
       + MPU_RASR_ENABLE_Msk },                       // region enable
    { 0U + 0x12U,                              //---- region #2
      0U }},
    {{ reinterpret_cast<std::uint32_t>(Philo_sto[4]) + 0x10U,//---- region #0
       ((PHILO_SIZE_POW2 - 1U) << MPU_RASR_SIZE_Pos)  // size
       + (3U << MPU_RASR_AP_Pos)                      // PA:rw/UA:rw
       + (1U << MPU_RASR_XN_Pos)                      // XN=1
       + (1U << MPU_RASR_S_Pos)                       // S=1
       + (1U << MPU_RASR_C_Pos)                       // C=1
       + (0U << MPU_RASR_B_Pos)                       // B=0
       + (0U << MPU_RASR_TEX_Pos)                     // TEX=0
       + MPU_RASR_ENABLE_Msk },                       // region enable
    { reinterpret_cast<std::uint32_t>(&Philo_shared_sto) + 0x11U,//---- region #1
      ((PHILO_SHARED_SIZE_POW2 - 1U) << MPU_RASR_SIZE_Pos)   // size
       + (3U << MPU_RASR_AP_Pos)                      // PA:rw/UA:rw
       + (1U << MPU_RASR_XN_Pos)                      // XN=1
       + (1U << MPU_RASR_S_Pos)                       // S=1
       + (1U << MPU_RASR_C_Pos)                       // C=1
       + (0U << MPU_RASR_B_Pos)                       // B=0
       + (0U << MPU_RASR_TEX_Pos)                     // TEX=0
       + MPU_RASR_ENABLE_Msk },                       // region enable
    { 0U + 0x12U,                              //---- region #2
      0U }},
};
#endif

// XThread1 thread............................................................
constexpr std::uint32_t XTHREAD1_SIZE_POW2  {10U};  // XThread1 instance + stack
constexpr std::uint32_t XTHREAD1_STACK_SIZE {400U}; // XThread1 stack size

__attribute__((aligned((1U << XTHREAD1_SIZE_POW2))))
std::uint8_t XThread1_sto[1U << XTHREAD1_SIZE_POW2];

static std::uint8_t * const XThread1_inst = &XThread1_sto[XTHREAD1_STACK_SIZE];

#ifdef QF_MEM_ISOLATE
static MPU_Region const MPU_XThread1[3] = {
    { reinterpret_cast<std::uint32_t>(XThread1_sto) + 0x10U,//---- region #0
      ((XTHREAD1_SIZE_POW2 - 1U) << MPU_RASR_SIZE_Pos) // size
       + (3U << MPU_RASR_AP_Pos)                      // PA:rw/UA:rw
       + (1U << MPU_RASR_XN_Pos)                      // XN=1
       + (1U << MPU_RASR_S_Pos)                       // S=1
       + (1U << MPU_RASR_C_Pos)                       // C=1
       + (0U << MPU_RASR_B_Pos)                       // B=0
       + (0U << MPU_RASR_TEX_Pos)                     // TEX=0
       + MPU_RASR_ENABLE_Msk },                       // region enable
    { GPIOA_BASE + 0x11U,                      //---- region #1
      ((9U - 1U) << MPU_RASR_SIZE_Pos)                // 2^9=512B size
       + (3U << MPU_RASR_AP_Pos)                      // PA:rw/UA:rw
       + (1U << MPU_RASR_XN_Pos)                      // XN=1
       + (1U << MPU_RASR_S_Pos)                       // S=1
       + (0U << MPU_RASR_C_Pos)                       // C=0
       + (1U << MPU_RASR_B_Pos)                       // B=1
       + (0U << MPU_RASR_TEX_Pos)                     // TEX=0
       + MPU_RASR_ENABLE_Msk },                       // region enable
    { 0U + 0x12U,                              //---- region #2
      0U },
};
#endif

// XThread2 thread............................................................
constexpr std::uint32_t XTHREAD2_SIZE_POW2  {10U};  // XThread2 instance + stack
constexpr std::uint32_t XTHREAD2_STACK_SIZE {400U}; // XThread2 stack size

__attribute__((aligned((1U << XTHREAD2_SIZE_POW2))))
std::uint8_t XThread2_sto[1U << XTHREAD2_SIZE_POW2];

static std::uint8_t * const XThread2_inst = &XThread2_sto[XTHREAD2_STACK_SIZE];

#ifdef QF_MEM_ISOLATE
static MPU_Region const MPU_XThread2[3] = {
    { reinterpret_cast<std::uint32_t>(XThread2_sto) + 0x10U,//---- region #0
      ((XTHREAD2_SIZE_POW2 - 1U) << MPU_RASR_SIZE_Pos) // size
       + (3U << MPU_RASR_AP_Pos)                      // PA:rw/UA:rw
       + (1U << MPU_RASR_XN_Pos)                      // XN=1
       + (1U << MPU_RASR_S_Pos)                       // S=1
       + (1U << MPU_RASR_C_Pos)                       // C=1
       + (0U << MPU_RASR_B_Pos)                       // B=0
       + (0U << MPU_RASR_TEX_Pos)                     // TEX=0
       + MPU_RASR_ENABLE_Msk },                       // region enable
    { GPIOA_BASE + 0x11U,                      //---- region #1
      ((9U - 1U) << MPU_RASR_SIZE_Pos)                // 2^9=512B size
       + (3U << MPU_RASR_AP_Pos)                      // PA:rw/UA:rw
       + (1U << MPU_RASR_XN_Pos)                      // XN=1
       + (1U << MPU_RASR_S_Pos)                       // S=1
       + (0U << MPU_RASR_C_Pos)                       // C=0
       + (1U << MPU_RASR_B_Pos)                       // B=1
       + (0U << MPU_RASR_TEX_Pos)                     // TEX=0
       + MPU_RASR_ENABLE_Msk },                       // region enable
    { 0U + 0x12U,                              //---- region #2
      0U },
};
#endif

// Shared Event-pools.........................................................
constexpr std::uint32_t EPOOLS_SIZE_POW2 {8U};

__attribute__((aligned((1U << EPOOLS_SIZE_POW2))))
static struct EPools {
    QF_MPOOL_EL(APP::TableEvt) smlPool[2*APP::N_PHILO];
    // ... other pools
} EPools_sto;
static_assert(sizeof(EPools_sto) <= (1U << EPOOLS_SIZE_POW2),
              "Insufficient storage for Event Pools");

//............................................................................
#ifdef QF_MEM_ISOLATE
static void STM32C031C6_MPU_setup(void) {

    MPU->CTRL = 0U; // disable the MPU

    // region #7: NULL-pointer protection region
    MPU->RBAR = 0x00000000U + 0x17U;        // base address + region #7
    MPU->RASR = ((8U - 1U) << MPU_RASR_SIZE_Pos) // 2^8=256B size
       + (0U << MPU_RASR_AP_Pos)                      // PA:na/UA:na
       + (1U << MPU_RASR_XN_Pos)                      // XN=1
       + (0U << MPU_RASR_S_Pos)                       // S=0
       + (0U << MPU_RASR_C_Pos)                       // C=0
       + (0U << MPU_RASR_B_Pos)                       // B=0
       + (0U << MPU_RASR_TEX_Pos)                     // TEX=0
       + MPU_RASR_ENABLE_Msk;                         // region enable

    // region #6: stack region
    MPU->RBAR = 0x20000000U + 0x16U;        // base address + region #6
    MPU->RASR = ((STACK_SIZE_POW2 - 1U) << MPU_RASR_SIZE_Pos)  // size
       + (3U << MPU_RASR_AP_Pos)                      // PA:rw/UA:rw
       + (1U << MPU_RASR_XN_Pos)                      // XN=1
       + (1U << MPU_RASR_S_Pos)                       // S=1
       + (1U << MPU_RASR_C_Pos)                       // C=1
       + (0U << MPU_RASR_B_Pos)                       // B=0
       + (0U << MPU_RASR_TEX_Pos)                     // TEX=0
       + MPU_RASR_ENABLE_Msk;                         // region enable

    // region #5: ROM region for STM32C031C6, whole 32K
    MPU->RBAR = 0x08000000U + 0x15U;        // base address + region #5
    MPU->RASR = ((15U - 1U) << MPU_RASR_SIZE_Pos)     // 2^15=32K size
       + (6U << MPU_RASR_AP_Pos)                      // PA:ro/UA:ro
       + (0U << MPU_RASR_XN_Pos)                      // XN=0
       + (0U << MPU_RASR_S_Pos)                       // S=0
       + (1U << MPU_RASR_C_Pos)                       // C=1
       + (0U << MPU_RASR_B_Pos)                       // B=0
       + (0U << MPU_RASR_TEX_Pos)                     // TEX=0
       + MPU_RASR_ENABLE_Msk;                         // region enable

    // region #4: Event-pools region
    MPU->RBAR = reinterpret_cast<std::uint32_t>(&EPools_sto) + 0x14U;// region #4
    MPU->RASR = ((EPOOLS_SIZE_POW2 - 1U) << MPU_RASR_SIZE_Pos)  // size
       + (3U << MPU_RASR_AP_Pos)                      // PA:rw/UA:rw
       + (1U << MPU_RASR_XN_Pos)                      // XN=1
       + (1U << MPU_RASR_S_Pos)                       // S=1
       + (1U << MPU_RASR_C_Pos)                       // C=1
       + (0U << MPU_RASR_B_Pos)                       // B=0
       + (0U << MPU_RASR_TEX_Pos)                     // TEX=0
       + MPU_RASR_ENABLE_Msk;                         // region enable

#ifdef Q_SPY
    // region #3: QS-filters region
    MPU->RBAR = reinterpret_cast<std::uint32_t>(&QP::QS::filt_) + 0x13U;// region #3
    MPU->RASR = ((8U - 1U) << MPU_RASR_SIZE_Pos)      // 2^8=256B size
       + (3U << MPU_RASR_AP_Pos)                      // PA:rw/UA:rw
       + (1U << MPU_RASR_XN_Pos)                      // XN=1
       + (1U << MPU_RASR_S_Pos)                       // S=1
       + (1U << MPU_RASR_C_Pos)                       // C=1
       + (0U << MPU_RASR_B_Pos)                       // B=0
       + (0U << MPU_RASR_TEX_Pos)                     // TEX=0
       + MPU_RASR_ENABLE_Msk;                         // region enable
#endif

    // region #0: temporary 4G region for the initial transient
    MPU->RBAR = 0x00000000U + 0x10U;        // base address + region #0
    MPU->RASR = ((32U - 1U) << MPU_RASR_SIZE_Pos) // 2^32=4G size
       + (3U << MPU_RASR_AP_Pos)                      // PA:rw/UA:rw
       + (0U << MPU_RASR_XN_Pos)                      // XN=0
       + (1U << MPU_RASR_S_Pos)                       // S=1
       + (1U << MPU_RASR_C_Pos)                       // C=1
       + (0U << MPU_RASR_B_Pos)                       // B=0
       + (0U << MPU_RASR_TEX_Pos)                     // TEX=0
       + MPU_RASR_ENABLE_Msk;                         // region enable

    MPU->CTRL = MPU_CTRL_ENABLE_Msk; // enable the MPU
                // but do NOT enable background region
    __ISB();
    __DSB();
}
#endif

} // unnamed namespace

//============================================================================
namespace APP {

// "opaque" pointer to AO
QP::QActive * const AO_Table = reinterpret_cast<QP::QActive *>(Table_sto);

QP::QActive * const AO_Philo[APP::N_PHILO] = {
    reinterpret_cast<QP::QActive *>(Philo_sto[0]), // "opaque" pointer
    reinterpret_cast<QP::QActive *>(Philo_sto[1]), // "opaque" pointer
    reinterpret_cast<QP::QActive *>(Philo_sto[2]), // "opaque" pointer
    reinterpret_cast<QP::QActive *>(Philo_sto[3]), // "opaque" pointer
    reinterpret_cast<QP::QActive *>(Philo_sto[4]), // "opaque" pointer
};

QP::QXThread * const TH_XThread1 =
    reinterpret_cast<QP::QXThread *>(XThread1_inst);
QP::QXThread * const TH_XThread2 =
    reinterpret_cast<QP::QXThread *>(XThread2_inst);

} // namespace APP

//============================================================================
namespace BSP {

void init() {
#ifdef QF_MEM_ISOLATE
    // setup the MPU
    STM32C031C6_MPU_setup();
#endif

    // NOTE: SystemInit() has been already called from the startup code
    // but SystemCoreClock needs to be updated
    SystemCoreClockUpdate();

    // enable GPIOA clock port for the LED LD4
    RCC->IOPENR |= (1U << 0U);

    // set all used GPIOA pins as push-pull output, no pull-up, pull-down
    GPIOA->MODER   &= ~(3U << 2U*LD4_PIN);
    GPIOA->MODER   |=  (1U << 2U*LD4_PIN);
    GPIOA->OTYPER  &= ~(1U <<    LD4_PIN);
    GPIOA->OSPEEDR &= ~(3U << 2U*LD4_PIN);
    GPIOA->OSPEEDR |=  (1U << 2U*LD4_PIN);
    GPIOA->PUPDR   &= ~(3U << 2U*LD4_PIN);

    // enable GPIOC clock port for the Button B1
    RCC->IOPENR |=  (1U << 2U);

    // configure Button B1 pin on GPIOC as input, no pull-up, pull-down
    GPIOC->MODER &= ~(3U << 2U*B1_PIN);
    GPIOC->PUPDR &= ~(3U << 2U*B1_PIN);

    BSP::randomSeed(1234U);

    // initialize the QS software tracing...
    if (!QS_INIT(nullptr)) {
        Q_ERROR();
    }

    // dictionaries...
    QS_OBJ_DICTIONARY(&l_SysTick_Handler);
    QS_OBJ_DICTIONARY(&l_EXTI0_1_IRQHandler);
    QS_USR_DICTIONARY(PHILO_STAT);
    QS_USR_DICTIONARY(PAUSED_STAT);

    QS_ONLY(APP::produce_sig_dict());
    QS_ONLY(APP::TH_obj_dict());

    // setup the QS filters...
    QS_GLB_FILTER(QP::QS_ALL_RECORDS);   // all records
    QS_GLB_FILTER(-QP::QS_QF_TICK);      // exclude the clock tick
    QS_LOC_FILTER(-(APP::N_PHILO + 3U)); // exclude prio. of AO_Ticker0
}
//............................................................................
void start() {
    // initialize event pools
    QP::QF::poolInit(EPools_sto.smlPool,
        sizeof(EPools_sto.smlPool),
        sizeof(EPools_sto.smlPool[0]));

    // initialize publish-subscribe
    static QP::QSubscrList subscrSto[APP::MAX_PUB_SIG];
    QP::QActive::psInit(subscrSto, Q_DIM(subscrSto));

    // instantiate and start AOs/threads...
    static QP::QEvt const *xThread1QueueSto[5];
#ifdef QF_MEM_ISOLATE
    APP::XThread1_ctor(XThread1_inst,
                       sizeof(XThread1_sto) - XTHREAD1_STACK_SIZE,
                       MPU_XThread1);
#else
    APP::XThread1_ctor(XThread1_inst,
                       sizeof(XThread1_sto) - XTHREAD1_STACK_SIZE);
#endif
    APP::TH_XThread1->start(
        1U,                          // QP priority of the thread
        xThread1QueueSto,            // event queue storage
        Q_DIM(xThread1QueueSto),     // event length [events]
        &XThread1_sto[0],            // stack storage
        XTHREAD1_STACK_SIZE);        // stack size [bytes]

    static QP::QEvt const *philoQueueSto[APP::N_PHILO][10];
    for (std::uint8_t n = 0U; n < APP::N_PHILO; ++n) {
#ifdef QF_MEM_ISOLATE
        APP::Philo_ctor(n, Philo_sto[n], sizeof(Philo_sto[n]), MPU_Philo[n]);
#else
        APP::Philo_ctor(n, Philo_sto[n], sizeof(Philo_sto[n]));
#endif
        APP::AO_Philo[n]->start(
            n + 3U,                  // QF-prio/pthre. see NOTE1
            philoQueueSto[n],        // event queue storage
            Q_DIM(philoQueueSto[n]), // queue length [events]
            nullptr, 0U);            // no stack storage
    }

    static QP::QEvt const *xThread2QueueSto[5];
#ifdef QF_MEM_ISOLATE
    APP::XThread2_ctor(XThread2_inst,
                       sizeof(XThread2_sto) - XTHREAD2_STACK_SIZE,
                       MPU_XThread2);
#else
    APP::XThread2_ctor(XThread2_inst,
                       sizeof(XThread2_sto) - XTHREAD2_STACK_SIZE);
#endif
    APP::TH_XThread2->start(
        APP::N_PHILO + 5U,           // QP priority of the thread
        xThread2QueueSto,            // event queue storage
        Q_DIM(xThread2QueueSto),     // event length [events]
        &XThread2_sto[0],            // stack storage
        XTHREAD2_STACK_SIZE);        // stack size [bytes]

    static QP::QEvt const *tableQueueSto[APP::N_PHILO];
#ifdef QF_MEM_ISOLATE
    APP::Table_ctor(Table_sto, sizeof(Table_sto), MPU_Table);
#else
    APP::Table_ctor(Table_sto, sizeof(Table_sto));
#endif
    APP::AO_Table->start(
        APP::N_PHILO + 7U,           // QP prio. of the AO
        tableQueueSto,               // event queue storage
        Q_DIM(tableQueueSto),        // queue length [events]
        nullptr, 0U);                // no stack storage
}
//............................................................................
void displayPhilStat(std::uint8_t n, char const *stat) {
    Q_UNUSED_PAR(n);

    if (stat[0] == 'e') {
        GPIOA->BSRR = (1U << LD4_PIN);  // turn LED on
    }
    else {
        GPIOA->BSRR = (1U << (LD4_PIN + 16U));  // turn LED off
    }

    // app-specific trace record...
    QS_BEGIN_ID(PHILO_STAT, APP::AO_Table->getPrio())
        QS_U8(1, n);  // Philosopher number
        QS_STR(stat); // Philosopher status
    QS_END()
}
//............................................................................
void displayPaused(std::uint8_t const paused) {
    // not enough LEDs to implement this feature
    if (paused != 0U) {
        //GPIOA->BSRR = (1U << LD4_PIN);  // turn LED[n] on
    }
    else {
        //GPIOA->BSRR = (1U << (LD4_PIN + 16U));  // turn LED[n] off
    }

    // application-specific trace record
    QS_BEGIN_ID(PAUSED_STAT, APP::AO_Table->getPrio())
        QS_U8(1, paused);  // Paused status
    QS_END()
}
//............................................................................
void randomSeed(std::uint32_t seed) {
    Philo_shared_sto.rnd_seed = seed;
}
//............................................................................
std::uint32_t random() { // a very cheap pseudo-random-number generator

    QP::QSchedStatus lockStat = QP::QXK::schedLock(APP::N_PHILO);
    // "Super-Duper" Linear Congruential Generator (LCG)
    // LCG(2^32, 3*7*11*13*23, 0, seed)
    //
    std::uint32_t rnd = Philo_shared_sto.rnd_seed * (3U*7U*11U*13U*23U);
    Philo_shared_sto.rnd_seed = rnd; // set for the next time
    QP::QXK::schedUnlock(lockStat);

    return (rnd >> 8U);
}
//............................................................................
void ledOn() {
    GPIOA->BSRR = (1U << LD4_PIN);  // turn LED on
}
//............................................................................
void ledOff() {
    GPIOA->BSRR = (1U << (LD4_PIN + 16U));  // turn LED off
}
//............................................................................
void terminate(int16_t result) {
    Q_UNUSED_PAR(result);
}

} // namespace BSP

//============================================================================
// namespace QP
namespace QP {

// QF callbacks...
void QF::onStartup() {
    // set up the SysTick timer to fire at BSP::TICKS_PER_SEC rate
    SysTick_Config(SystemCoreClock / BSP::TICKS_PER_SEC);

    // assign all priority bits for preemption-prio. and none to sub-prio.
    // NOTE: this might have been changed by STM32Cube.
    NVIC_SetPriorityGrouping(0U);

    // set priorities of ALL ISRs used in the system, see NOTE1
    NVIC_SetPriority(USART2_IRQn,    0U); // kernel UNAWARE interrupt
    NVIC_SetPriority(EXTI0_1_IRQn,   QF_AWARE_ISR_CMSIS_PRI + 0U);
    NVIC_SetPriority(SysTick_IRQn,   QF_AWARE_ISR_CMSIS_PRI + 1U);
    // ...

    // enable IRQs...
    NVIC_EnableIRQ(EXTI0_1_IRQn);

#ifdef Q_SPY
    NVIC_EnableIRQ(USART2_IRQn); // UART2 interrupt used for QS-RX
#endif
}
//............................................................................
void QF::onCleanup() {
}
//............................................................................
void QXK::onIdle() {

    // toggle an LED on and then off (not enough LEDs, see NOTE02)
    //QF_INT_DISABLE();
    //QF_MEM_SYS();
    //GPIOA->BSRR = (1U << LD4_PIN);         // turn LED[n] on
    //GPIOA->BSRR = (1U << (LD4_PIN + 16U)); // turn LED[n] off
    //QF_MEM_APP();
    //QF_INT_ENABLE();

#ifdef Q_SPY
    QF_INT_DISABLE();
    QF_MEM_SYS();
    QS::rxParse();  // parse all the received bytes
    QF_MEM_APP();
    QF_INT_ENABLE();
    QF_CRIT_EXIT_NOP();

    QF_INT_DISABLE();
    QF_MEM_SYS();
    if ((USART2->ISR & (1U << 7U)) != 0U) { // is TXE empty?
        std::uint16_t b = QS::getByte();

        if (b != QS_EOD) {   // not End-Of-Data?
            USART2->TDR = b; // put into the DR register
        }
    }
    QF_MEM_APP();
    QF_INT_ENABLE();
#elif defined NDEBUG
    // Put the CPU and peripherals to the low-power mode.
    // you might need to customize the clock management for your application,
    // see the datasheet for your particular Cortex-M MCU.
    __WFI(); // Wait-For-Interrupt
#endif
}

//============================================================================
// QS facilities...
#ifdef Q_SPY
namespace QS {

#ifdef QF_MEM_ISOLATE
// Shared QS filters...
__attribute__((aligned(32)))
Filter filt_;
#endif

//............................................................................
static uint16_t const UARTPrescTable[12] = {
    1U, 2U, 4U, 6U, 8U, 10U, 12U, 16U, 32U, 64U, 128U, 256U
};

#define UART_DIV_SAMPLING16(__PCLK__, __BAUD__, __CLOCKPRESCALER__) \
  ((((__PCLK__)/UARTPrescTable[(__CLOCKPRESCALER__)]) \
  + ((__BAUD__)/2U)) / (__BAUD__))

#define UART_PRESCALER_DIV1  0U

// USART2 pins PA.2 and PA.3
#define USART2_TX_PIN 2U
#define USART2_RX_PIN 3U

//............................................................................
bool onStartup(void const *arg) {
    Q_UNUSED_PAR(arg);

    static std::uint8_t qsTxBuf[2*1024]; // buffer for QS transmit channel
    initBuf(qsTxBuf, sizeof(qsTxBuf));

    static std::uint8_t qsRxBuf[100];    // buffer for QS receive channel
    rxInitBuf(qsRxBuf, sizeof(qsRxBuf));

    // enable peripheral clock for USART2
    RCC->IOPENR  |= ( 1U <<  0U);  // Enable GPIOA clock for USART pins
    RCC->APBENR1 |= ( 1U << 17U);  // Enable USART#2 clock

    // Configure PA to USART2_RX, PA to USART2_TX
    GPIOA->AFR[0] &= ~((15U << 4U*USART2_RX_PIN) | (15U << 4U*USART2_TX_PIN));
    GPIOA->AFR[0] |=  (( 1U << 4U*USART2_RX_PIN) | ( 1U << 4U*USART2_TX_PIN));
    GPIOA->MODER  &= ~(( 3U << 2U*USART2_RX_PIN) | ( 3U << 2U*USART2_TX_PIN));
    GPIOA->MODER  |=  (( 2U << 2U*USART2_RX_PIN) | ( 2U << 2U*USART2_TX_PIN));

    // baud rate
    USART2->BRR  = UART_DIV_SAMPLING16(
                       SystemCoreClock, 115200U, UART_PRESCALER_DIV1);
    USART2->CR3  = 0x0000U |      // no flow control
                   (1U << 12U);   // disable overrun detection (OVRDIS)
    USART2->CR2  = 0x0000U;       // 1 stop bit
    USART2->CR1  = ((1U <<  2U) | // enable RX
                    (1U <<  3U) | // enable TX
                    (1U <<  5U) | // enable RX interrupt
                    (0U << 12U) | // 8 data bits
                    (0U << 28U) | // 8 data bits
                    (1U <<  0U)); // enable USART

    QS_tickPeriod_ = SystemCoreClock / BSP::TICKS_PER_SEC;
    QS_tickTime_ = QS_tickPeriod_; // to start the timestamp at zero

    return true; // return success
}
//............................................................................
void onCleanup() {
}
//............................................................................
QSTimeCtr onGetTime() { // NOTE: invoked with interrupts DISABLED
    if ((SysTick->CTRL & 0x00010000U) == 0U) { // not set?
        return QS_tickTime_ - (QSTimeCtr)SysTick->VAL;
    }
    else { // the rollover occured, but the SysTick_ISR did not run yet
        return QS_tickTime_ + QS_tickPeriod_ - (QSTimeCtr)SysTick->VAL;
    }
}
//............................................................................
// NOTE:
// No critical section in QS::onFlush() to avoid nesting of critical sections
// in case QS::onFlush() is called from Q_onError().
void onFlush() {
    for (;;) {
        std::uint16_t b = getByte();
        if (b != QS_EOD) {
            while ((USART2->ISR & (1U << 7U)) == 0U) { // while TXE not empty
            }
            USART2->TDR = b; // put into the DR register
        }
        else {
            break;
        }
    }
}
//............................................................................
void onReset() {
    NVIC_SystemReset();
}
//............................................................................
void onCommand(std::uint8_t cmdId, std::uint32_t param1,
               std::uint32_t param2, std::uint32_t param3)
{
    Q_UNUSED_PAR(cmdId);
    Q_UNUSED_PAR(param1);
    Q_UNUSED_PAR(param2);
    Q_UNUSED_PAR(param3);
}

} // namespace QS
#endif // Q_SPY
//----------------------------------------------------------------------------

} // namespace QP

//============================================================================
// NOTE1:
// The QF_AWARE_ISR_CMSIS_PRI constant from the QF port specifies the highest
// ISR priority that is disabled by the QF framework. The value is suitable
// for the NVIC_SetPriority() CMSIS function.
//
// Only ISRs prioritized at or below the QF_AWARE_ISR_CMSIS_PRI level (i.e.,
// with the numerical values of priorities equal or higher than
// QF_AWARE_ISR_CMSIS_PRI) are allowed to call the QK_ISR_ENTRY/
// QXK_ISR_ENTRY macros or any other QF/QXK services. These ISRs are
// "QF-aware".
//
// Conversely, any ISRs prioritized above the QF_AWARE_ISR_CMSIS_PRI priority
// level (i.e., with the numerical values of priorities less than
// QF_AWARE_ISR_CMSIS_PRI) are never disabled and are not aware of the kernel.
// Such "QF-unaware" ISRs cannot call ANY QF/QXK services. In particular they
// can NOT call the macros QXK_ISR_ENTRY/QXK_ISR_ENTRY. The only mechanism
// by which a "QF-unaware" ISR can communicate with the QF framework is by
// triggering a "QF-aware" ISR, which can post/publish events.
//
// NOTE2:
// The User LED is used to visualize the idle loop activity. The brightness
// of the LED is proportional to the frequency of the idle loop.
// Please note that the LED is toggled with interrupts locked, so no interrupt
// execution time contributes to the brightness of the User LED.
//
