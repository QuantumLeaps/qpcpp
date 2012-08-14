//////////////////////////////////////////////////////////////////////////////
// Product: DPP example, QK version, Open Watcom
// Last Updated for Version: 4.5.00
// Date of the Last Update:  May 20, 2012
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
#include "video.h"

#include <math.h>                                           // to test the FPU
#include <dos.h>                          // for _dos_setvect()/_dos_getvect()
#include <conio.h>                                         // for inp()/outp()
#include <stdlib.h>                                             // for _exit()

Q_DEFINE_THIS_FILE

// Global-scope objects ------------------------------------------------------
Lib1_context * volatile impure_ptr1;
Lib2_context * volatile impure_ptr2;

// Local-scope objects -------------------------------------------------------
static void interrupt (*l_dosTmrISR)(void);
static void interrupt (*l_dosKbdISR)(void);
static uint32_t l_delay = 0UL;    // limit for the loop counter in busyDelay()

#ifdef Q_SPY
    static uint16_t  l_uart_base;          // QS data uplink UART base address
    static QSTimeCtr l_tickTime;                    // keeps timetsamp at tick
    static uint8_t   l_tmr;
    static uint8_t   l_kbd;

    #define UART_16550_TXFIFO_DEPTH 16

    enum AppRecords {                    // application-specific trace records
        PHILO_STAT = QS_USER
    };
#endif

#define TMR_VECTOR      0x08
#define KBD_VECTOR      0x09

#define TMR_ISR_PRIO    (0xFF)
#define KBD_ISR_PRIO    (0xFF - 1)

#define M_PI            3.14159265358979323846264338327950288

static void dispPreemptions(uint8_t pisr);          // for testing, see NOTE01
static void busyDelay(void);                        // for testing, see NOTE02

//............................................................................
void interrupt ISR_tmr(void) {
    dispPreemptions(TMR_ISR_PRIO);                 // for testing only, NOTE01

    QK_ISR_ENTRY();                        // inform QK about entering the ISR

    QF::TICK(&l_tmr);            // call QF_tick() outside of critical section
#ifdef Q_SPY
    l_tickTime += 0x10000;                              // add 16-bit rollover
#endif

    busyDelay();                                        // for testing, NOTE02
    QK_ISR_EXIT();                          // inform QK about exiting the ISR
}
//............................................................................
void interrupt ISR_kbd(void) {
    dispPreemptions(KBD_ISR_PRIO);                 // for testing only, NOTE01

    QK_ISR_ENTRY();                        // inform QK about entering the ISR

    uint8_t key = inp(0x60);          // key code from the 8042 kbd controller
    uint8_t kcr = inp(0x61);                  // get keyboard control register
    outp(0x61, (uint8_t)(kcr | 0x80));          // toggle acknowledge bit high
    outp(0x61, kcr);                             // toggle acknowledge bit low
    if (key == (uint8_t)129) {                             // ESC key pressed?
        static QEvt term = {TERMINATE_SIG, 0};               // static event
        QF::PUBLISH(&term, &l_kbd);           // publish to all interested AOs
    }
    else {
        static QEvt test = {TEST_SIG, 0};                    // static event
        AO_Table->POST(&test, &l_kbd);           // post a test event to Table
    }
    Video::printNumAt(60, 12 + 0, Video::FGND_YELLOW, key);     // display key

    busyDelay();                                        // for testing, NOTE02
    QK_ISR_EXIT();                          // inform QK about exiting the ISR
}
//............................................................................
void QF::onStartup(void) {
                                         // save the origingal DOS vectors ...
    l_dosTmrISR = _dos_getvect(TMR_VECTOR);
    l_dosKbdISR = _dos_getvect(KBD_VECTOR);

    QF_INT_DISABLE();
    _dos_setvect(TMR_VECTOR, &ISR_tmr);
    _dos_setvect(KBD_VECTOR, &ISR_kbd);
    QF_INT_ENABLE();
}
//............................................................................
void QF::onCleanup(void) {             // restore the original DOS vectors ...
    QF_INT_DISABLE();
    _dos_setvect(TMR_VECTOR, l_dosTmrISR);
    _dos_setvect(KBD_VECTOR, l_dosKbdISR);
    QF_INT_ENABLE();

    QS_EXIT();                                                      // exit QS
    _exit(0);                                                   // exit to DOS
}
   //.........................................................................
void QK::onIdle(void) {
#ifdef Q_SPY
    if ((inp(l_uart_base + 5) & (1 << 5)) != 0) {            // Tx FIFO empty?
        uint16_t fifo = UART_16550_TXFIFO_DEPTH;        // 16550 Tx FIFO depth
        uint8_t const *block;
        QF_INT_DISABLE();
        block = QS::getBlock(&fifo);      // try to get next block to transmit
        QF_INT_ENABLE();
        while (fifo-- != 0) {                       // any bytes in the block?
            outp(l_uart_base + 0, *block++);
        }
    }
#endif
}
//............................................................................
void BSP_init(int argc, char *argv[]) {
    if (argc > 1) {
        l_delay = atol(argv[1]);       // set the delay counter for busy delay
    }

    char const *com = "COM1";
    if (argc > 2) {
        com = argv[2];
        com = com;         // avoid the compiler warning about unused variable
    }
    if (!QS_INIT(com)) {                                      // initialize QS
        Q_ERROR();
    }

    QS_OBJ_DICTIONARY(&l_tmr);
    QS_OBJ_DICTIONARY(&l_kbd);

    uint8_t n;
    Video::clearScreen(Video::BGND_BLACK);
    Video::clearRect( 0,  0, 80,  7, Video::BGND_LIGHT_GRAY);
    Video::clearRect( 0, 11, 80, 12, Video::BGND_LIGHT_GRAY);
    Video::clearRect( 0, 12, 41, 23, Video::BGND_BLUE);
    Video::clearRect(41, 12, 80, 23, Video::BGND_RED);
    Video::clearRect( 0, 23, 80, 24, Video::BGND_LIGHT_GRAY);

    n = Video::FGND_BLUE;
    Video::printStrAt(10, 0, n, "  __");
    Video::printStrAt(10, 1, n, " /  |      _   _ -|-     _ _");
    Video::printStrAt(10, 2, n, " \\__| | |  _\\ | \\ | | | | \\ \\");
    Video::printStrAt(10, 3, n, "    | \\_/ |_| | | | \\_| | | |");
    Video::printStrAt(10, 4, n, "    |");
    n = Video::FGND_RED;
    Video::printStrAt(43, 0, n, "    _       __ ");
    Video::printStrAt(43, 1, n, "|  /_\\     |  \\  TM");
    Video::printStrAt(43, 2, n, "|  \\_   _  |__/ _");
    Video::printStrAt(43, 3, n, "|       _\\ |   |_");
    Video::printStrAt(43, 4, n, "|___   |_| |    _|");
    Video::printStrAt(10, 5, Video::FGND_BLUE,
                     "_____________________________________________________");
    Video::printStrAt(10, 6, Video::FGND_RED,
                     "i n n o v a t i n g   e m b e d d e d   s y s t e m s");
    Video::printStrAt(18,  7, Video::FGND_WHITE,
                     "Dining Philosophers Problem (DPP)");
    Video::printStrAt(18,  8, Video::FGND_WHITE, "QEP/C++");
    Video::printStrAt(28,  8, Video::FGND_YELLOW, QEP::getVersion());
    Video::printStrAt(18,  9, Video::FGND_WHITE, "QF/C++");
    Video::printStrAt(28,  9, Video::FGND_YELLOW, QF::getVersion());
    Video::printStrAt(18, 10, Video::FGND_WHITE, "QK/C++");
    Video::printStrAt(28, 10, Video::FGND_YELLOW, QK::getVersion());
    Video::printStrAt(41, 10, Video::FGND_WHITE, "Delay Counter");
    Video::printNumAt(56, 10, Video::FGND_YELLOW, l_delay);

    Video::printStrAt( 1, 11, Video::FGND_BLUE,
                     "Active Object   State     Preemptions");

    Video::printStrAt(42, 11, Video::FGND_RED,
                     "ISR      Calls    Data    Preemptions");
    for (n = 0; n < N_PHILO; ++n) {
        Video::printStrAt( 1, 12 + n, Video::FGND_WHITE, "Philosopher");
        Video::printNumAt(12, 12 + n, Video::FGND_WHITE, n);
    }
    Video::printStrAt( 1, 12 + N_PHILO, Video::FGND_WHITE,  "Table");
    Video::printStrAt(17, 12 + N_PHILO, Video::FGND_YELLOW, "serving");

    Video::printStrAt(42, 12 + 0, Video::FGND_WHITE,  "kbdISR");
    Video::printStrAt(42, 12 + 1, Video::FGND_WHITE,  "tmrISR");

    Video::printStrAt(10, 23, Video::FGND_BLUE,
         "* Copyright (c) Quantum Leaps, LLC * www.quantum-leaps.com *");
    Video::printStrAt(28, 24, Video::FGND_LIGHT_RED,
         "<< Press Esc to quit >>");
}
//............................................................................
void BSP_displyPhilStat(uint8_t n, char const *stat) {
    Video::printStrAt(17, 12 + n, Video::FGND_YELLOW, stat);

    QS_BEGIN(PHILO_STAT, AO_Philo[n])     // application-specific record begin
        QS_U8(1, n);                                     // Philosopher number
        QS_STR(stat);                                    // Philosopher status
    QS_END()
}
//............................................................................
void Q_onAssert(char const Q_ROM * const Q_ROM_VAR file, int line) {
    Video::clearRect(0, 24, 80, 25, Video::BGND_RED);
    Video::printStrAt(0, 24, Video::FGND_WHITE, "ASSERTION FAILED in file:");
    Video::printStrAt(26, 24, Video::FGND_YELLOW, file);
    Video::printStrAt(57, 24, Video::FGND_WHITE, "line:");
    Video::printNumAt(62, 24, Video::FGND_YELLOW, line);
    QF::stop();
}
//............................................................................
void busyDelay(void) {                                          // for testing
    uint32_t volatile i = l_delay << 4;
    while (i-- > 0UL) {                                      // busy-wait loop
    }
}
//............................................................................
void dispPreemptions(uint8_t pisr) {                // for testing, see NOTE01
    if (pisr == TMR_ISR_PRIO) {
        static uint32_t tmrIsrCtr;                  // timer interrupt counter
        Video::printNumAt(51, 12 + 1, Video::FGND_YELLOW, ++tmrIsrCtr);
    }
    else if (pisr == KBD_ISR_PRIO) {
        static uint32_t kbdIsrCtr;                    // kbd interrupt counter
        Video::printNumAt(51, 12 + 0, Video::FGND_YELLOW, ++kbdIsrCtr);
    }
    else {
        Q_ERROR();                            // unexpected interrupt priority
    }

    if (QK_intNest_ == (uint8_t)0) {             // is this a task preemption?
        if (QK_currPrio_ > (uint8_t)0) {
            static uint32_t preCtr[QF_MAX_ACTIVE + 1];
            Video::printNumAt(30, 12 + QK_currPrio_ - 1, Video::FGND_YELLOW,
                             ++preCtr[QK_currPrio_]);
        }
    }
    else if (QK_intNest_ == (uint8_t)1) {         // this is an ISR preemption
        if (pisr == TMR_ISR_PRIO) {             // TMR_ISR preempting KBD_ISR?
            static uint32_t kbdPreCtr;           // kbd ISR preemption counter
            Video::printNumAt(71, 12 + 0, Video::FGND_YELLOW, ++kbdPreCtr);
        }
        else {
            static uint32_t tmrPreCtr;           // tmr ISR preemption counter
            Video::printNumAt(71, 12 + 1, Video::FGND_YELLOW, ++tmrPreCtr);
        }
    }
    else {
        Q_ERROR();            // impossible ISR nesting level with just 2 ISRs
    }
}

//----------------------------------------------------------------------------
void lib1_reent_init(uint8_t prio) {
    impure_ptr1->x = (double)prio * (M_PI / 6.0);
}
//............................................................................
void lib1_test(void) {
    uint32_t volatile i = l_delay;
    while (i-- > 0UL) {
        double volatile r = sin(impure_ptr1->x) * sin(impure_ptr1->x)
                            + cos(impure_ptr1->x) * cos(impure_ptr1->x);
        Q_ASSERT(fabs(r - 1.0) < 1e-99);
    }
}
//----------------------------------------------------------------------------
void lib2_reent_init(uint8_t prio) {
    impure_ptr2->y = (double)prio * (M_PI / 6.0) + M_PI;
}
//............................................................................
void lib2_test(void) {
    uint32_t volatile i = l_delay;
    while (i-- > 0UL) {
        double volatile r = sin(impure_ptr2->y) * sin(impure_ptr2->y)
                            + cos(impure_ptr2->y) * cos(impure_ptr2->y);
        Q_ASSERT(fabs(r - 1.0) < 1e-99);
    }
}

//----------------------------------------------------------------------------
#ifdef Q_SPY                                            // define QS callbacks

//............................................................................
static bool UART_config(char const *comName, uint32_t baud) {
    switch (comName[3]) {             // Set the base address of the COMx port
        case '1': l_uart_base = (uint16_t)0x03F8; break;               // COM1
        case '2': l_uart_base = (uint16_t)0x02F8; break;               // COM2
        case '3': l_uart_base = (uint16_t)0x03E8; break;               // COM3
        case '4': l_uart_base = (uint16_t)0x02E8; break;               // COM4
        default: return (uint8_t)0;           // COM port out of range failure
    }
    baud = (uint16_t)(115200UL / baud);               // divisor for baud rate
    outp(l_uart_base + 3, (1 << 7));          // Set divisor access bit (DLAB)
    outp(l_uart_base + 0, (uint8_t)baud);                      // Load divisor
    outp(l_uart_base + 1, (uint8_t)(baud >> 8));
    outp(l_uart_base + 3, (1 << 1) | (1 << 0));       // LCR:8-bits,no p,1stop
    outp(l_uart_base + 4, (1 << 3) | (1 << 1) | (1 << 0));      // DTR,RTS,Out
    outp(l_uart_base + 1, 0);           // Put UART into the polling FIFO mode
    outp(l_uart_base + 2, (1 << 2) | (1 << 0));       // FCR: enable, TX clear

    return true;                                                    // success
}
//............................................................................
bool QS::onStartup(void const *arg) {
    static uint8_t qsBuf[1*1024];                    // buffer for Quantum Spy
    initBuf(qsBuf, sizeof(qsBuf));
    return UART_config((char const *)arg, 115200UL);
}
//............................................................................
void QS::onCleanup(void) {
}
//............................................................................
QSTimeCtr QS::onGetTime(void) {              // invoked with interrupts locked
    static uint32_t l_lastTime;
    uint32_t now;
    uint16_t count16;                            // 16-bit count from the 8254

    outp(0x43, 0);                         // latch the 8254's counter-0 count
    count16 = (uint16_t)inp(0x40);           // read the low byte of counter-0
    count16 += ((uint16_t)inp(0x40) << 8);               // add on the hi byte

    now = l_tickTime + (0x10000 - count16);

    if (l_lastTime > now) {                    // are we going "back" in time?
        now += 0x10000;                  // assume that there was one rollover
    }
    l_lastTime = now;

    return (QSTimeCtr)now;
}
//............................................................................
void QS::onFlush(void) {
    uint16_t fifo = UART_16550_TXFIFO_DEPTH;            // 16550 Tx FIFO depth
    uint8_t const *block;
    while ((block = getBlock(&fifo)) != (uint8_t *)0) {
                                              // busy-wait until TX FIFO empty
        while ((inp(l_uart_base + 5) & (1 << 5)) == 0) {
        }

        while (fifo-- != 0) {                       // any bytes in the block?
            outp(l_uart_base + 0, *block++);
        }
        fifo = UART_16550_TXFIFO_DEPTH;         // re-load 16550 Tx FIFO depth
    }
}
#endif                                                                // Q_SPY
//----------------------------------------------------------------------------


//////////////////////////////////////////////////////////////////////////////
// NOTE01:
// The function call to displayPreemptions() is added only to monitor the
// "asynchronous" preemptions within the QK.
//
// NOTE02:
// The call to busyDelay() is added only to extend the execution
// time of the code to increase the chance of an "asynchronous" preemption.
//
