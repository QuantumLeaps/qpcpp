#include "qpc.h"
#include "dpp.h"
#include "bsp.h"

namespace DPP {

//............................................................................
int_t main(void) {
    // statically allocated storage for various QP facilities
    static QP::QEvt const *tableQueueSto[N_PHILO];
    static QP::QEvt const *philoQueueSto[N_PHILO][N_PHILO];
    static QP::QSubscrList subscrSto[MAX_PUB_SIG];
    static QF_MPOOL_EL(TableEvt) smlPoolSto[2*N_PHILO];

    QP::QF::init();   // initialize the framework and the underlying RT kernel

    BSP::init(); // initialize the BSP

    // object dictionaries...
    QS_OBJ_DICTIONARY(smlPoolSto);
    QS_OBJ_DICTIONARY(tableQueueSto);
    QS_OBJ_DICTIONARY(philoQueueSto[0]);
    QS_OBJ_DICTIONARY(philoQueueSto[1]);
    QS_OBJ_DICTIONARY(philoQueueSto[2]);
    QS_OBJ_DICTIONARY(philoQueueSto[3]);
    QS_OBJ_DICTIONARY(philoQueueSto[4]);

    QP::QF::psInit(subscrSto, Q_DIM(subscrSto)); // init publish-subscribe

    // initialize event pools...
    QP::QF::poolInit(smlPoolSto,
                     sizeof(smlPoolSto), sizeof(smlPoolSto[0]));

    // start the active objects...
    for (uint8_t n = 0U; n < N_PHILO; ++n) {
        AO_Philo[n]->start((uint8_t)(n + 1U),
                           philoQueueSto[n], Q_DIM(philoQueueSto[n]),
                           (void *)0, 0U);
    }
    AO_Table->start((uint8_t)(N_PHILO + 1U),
                    tableQueueSto, Q_DIM(tableQueueSto),
                    (void *)0, 0U);

    return QP::QF::run(); // run the QF application
}

} // namespace DPP
