#include "qpc.h"
#include "dpp.h"
#include "bsp.h"

namespace DPP {

//............................................................................
int_t main(void) {

    QP::QF::init();   // initialize the framework and the underlying RT kernel
    BSP::init(); // initialize the BSP

    // object dictionaries...
    QS_OBJ_DICTIONARY(AO_Table);
    QS_OBJ_DICTIONARY(AO_Philo[0]);
    QS_OBJ_DICTIONARY(AO_Philo[1]);
    QS_OBJ_DICTIONARY(AO_Philo[2]);
    QS_OBJ_DICTIONARY(AO_Philo[3]);
    QS_OBJ_DICTIONARY(AO_Philo[4]);

    static QP::QSubscrList subscrSto[MAX_PUB_SIG];
    QP::QActive::psInit(subscrSto, Q_DIM(subscrSto)); // init publish-subscribe

    // initialize event pools...
    static QF_MPOOL_EL(TableEvt) smlPoolSto[2*N_PHILO];
    QP::QF::poolInit(smlPoolSto,
                     sizeof(smlPoolSto), sizeof(smlPoolSto[0]));

    // start the active objects...
    static QP::QEvt const *philoQueueSto[N_PHILO][N_PHILO];
    for (std::uint8_t n = 0U; n < N_PHILO; ++n) {
        AO_Philo[n]->start(n + 1U,
                           philoQueueSto[n], Q_DIM(philoQueueSto[n]),
                           nullptr, 0U);
    }

    static QP::QEvt const *tableQueueSto[N_PHILO];
    AO_Table->start(N_PHILO + 1U,
                    tableQueueSto, Q_DIM(tableQueueSto),
                    nullptr, 0U);

    return QP::QF::run(); // run the QF application
}

} // namespace DPP
