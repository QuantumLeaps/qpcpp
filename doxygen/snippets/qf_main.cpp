#include "qpcpp.hpp"
#include "dpp.hpp"
#include "bsp.hpp"

namespace DPP {

//............................................................................
int_t main(void) {

    QP::QF::init();   // initialize the framework and the underlying RT kernel
    BSP::init(); // initialize the BSP

    // object dictionaries...
    QS_OBJ_DICTIONARY(AO_Table);
    for (std::uint8_t n = 0U; n < N_PHILO; ++n) {
        QS_OBJ_ARR_DICTIONARY(AO_Philo[n], n);
    }

    // initialize publish-subscribe...
    static QP::QSubscrList subscrSto[MAX_PUB_SIG];
    QP::QActive::psInit(subscrSto, Q_DIM(subscrSto));

    // initialize event pools...
    static QF_MPOOL_EL(TableEvt) smlPoolSto[2*N_PHILO];
    QP::QF::poolInit(smlPoolSto,
                     sizeof(smlPoolSto), sizeof(smlPoolSto[0]));

    // start the active objects...
    static QP::QEvt const *philoQueueSto[N_PHILO][N_PHILO];
    for (std::uint8_t n = 0U; n < N_PHILO; ++n) {
        AO_Philo[n]->start(
            Q_PRIO(n + 1U, N_PHILO), // QF-priority/preemption-threshold
            philoQueueSto[n],        // event queue storage
            Q_DIM(philoQueueSto[n]), // event queue length [events]
            nullptr,                 // stack storage (not used)
            0U);                     // stack size [bytes] (not used)
    }

    static QP::QEvt const *tableQueueSto[N_PHILO];
    AO_Table->start(
        N_PHILO + 1U,
        tableQueueSto,
        Q_DIM(tableQueueSto),
        nullptr,
        0U);

    return QP::QF::run(); // run the QF application
}

} // namespace DPP
