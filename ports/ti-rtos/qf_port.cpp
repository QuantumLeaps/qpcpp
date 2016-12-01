/// @file
/// @brief QF/C++ port to TI-RTOS kernel, all supported compilers
/// @cond
////**************************************************************************
/// Last updated for version 5.8.0
/// Last updated on  2016-11-19
///
///                    Q u a n t u m     L e a P s
///                    ---------------------------
///                    innovating embedded systems
///
/// Copyright (C) Quantum Leaps, LLC. All rights reserved.
///
/// This program is open source software: you can redistribute it and/or
/// modify it under the terms of the GNU General Public License as published
/// by the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
///
/// Alternatively, this program may be distributed and modified under the
/// terms of Quantum Leaps commercial licenses, which expressly supersede
/// the GNU General Public License and are specifically designed for
/// licensees interested in retaining the proprietary status of their code.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program. If not, see <http://www.gnu.org/licenses/>.
///
/// Contact information:
/// http://www.state-machine.com
/// mailto:info@state-machine.com
////**************************************************************************
/// @endcond

#define QP_IMPL           // this is QP implementation
#include "qf_port.h"      // QF port
#include "qf_pkg.h"
#include "qassert.h"
#ifdef Q_SPY              // QS software tracing enabled?
    #include "qs_port.h"  // include QS port
#else
    #include "qs_dummy.h" // disable the QS software tracing
#endif // Q_SPY

#include <ti/sysbios/BIOS.h> // SYS/BIOS API (for BIOS_start())

// namespace QP ==============================================================
namespace QP {

Q_DEFINE_THIS_MODULE("qf_port")

// Local objects -------------------------------------------------------------
static void swi_function(UArg arg0, UArg arg1); // TI-RTOS Swi signature

//............................................................................
void QF::init(void) {
}
//............................................................................
int_t QF::run(void) {
    onStartup();     // configure & start interrupts, see NOTE0
    BIOS_start();    // start TI-RTOT (SYS/BIOS)

    Q_ERROR_ID(100); // BIOS_start() should never return
    return static_cast<int_t>(0); // dummy return to make the compiler happy
}
//............................................................................
void QF::stop(void) {
    onCleanup();  // cleanup callback
}

//............................................................................
void QActive::start(uint_fast8_t prio,
                    QEvt const *qSto[], uint_fast16_t qLen,
                    void *stkSto, uint_fast16_t /* stkSize */,
                    QEvt const *ie)
{
    /// @pre the priority must be in range and the stack storage must not
    /// be provided, because this TI-RTOS port does not need per-AO stacks.
    ///
    Q_REQUIRE_ID(400, (static_cast<uint_fast8_t>(0) < prio)
              && (prio <= static_cast<uint_fast8_t>(QF_MAX_ACTIVE))
              && (stkSto == static_cast<void *>(0)));


    m_eQueue.init(qSto, qLen); // initialize QEQueue of this AO
    m_prio = prio;  // set QF priority of this AO before adding it to QF
    QF::add_(this); // make QF aware of this active object
    init(ie);       // thake the top-most initial tran.
    QS_FLUSH();     // flush the trace buffer to the host

    // create TI-RTOS Swi to run this active object...
    Swi_Params swiParams;
    Swi_Params_init(&swiParams);
    swiParams.arg0 = (xdc_UArg)this; // the "me" pointer
    swiParams.arg1 = 0;        // not used
    swiParams.priority = prio; // TI-RTOS Swis can use the QP priority
    swiParams.trigger = 0;     // not used
    Swi_construct(&m_osObject, &swi_function, &swiParams, NULL);
    m_thread = Swi_handle(&m_osObject);

    // TI-RTOS Swi must be created correctly
    Q_ENSURE_ID(490, m_thread != 0);
}
//............................................................................
void QActive::stop() {
    QF::remove_(this); // remove the AO from the framework
}

//............................................................................
static void swi_function(UArg arg0, UArg /* arg1 */) { // TI-RTOS Swi
    QActive *act = reinterpret_cast<QActive *>(arg0);
    QEvt const *e = act->get_();
    act->dispatch(e);
    QF::gc(e);

    // are events still available for this AO?
    if (!act->m_eQueue.isEmpty()) {
        Swi_post(act->m_thread);
    }
}

} // namespace QP

///***************************************************************************
//
