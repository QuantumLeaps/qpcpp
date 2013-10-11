#include "qp_port.h"
#include "dpp.h"
#include "bsp.h"

namespace DPP {

// Local-scope objects -------------------------------------------------------
static QP::QEvt const *l_tableQueueSto[N_PHILO];
static QP::QEvt const *l_philoQueueSto[N_PHILO][N_PHILO];
static QP::QSubscrList l_subscrSto[MAX_PUB_SIG];

static QF_MPOOL_EL(TableEvt) l_smlPoolSto[2*N_PHILO];//storage for small epool

//............................................................................
extern "C" int_t main(void) {

    QP::QF::init();   // initialize the framework and the underlying RT kernel

    BSP_init();                                          // initialize the BSP

                                                     // object dictionaries...
    QS_OBJ_DICTIONARY(l_smlPoolSto);
    QS_OBJ_DICTIONARY(l_tableQueueSto);
    QS_OBJ_DICTIONARY(l_philoQueueSto[0]);
    QS_OBJ_DICTIONARY(l_philoQueueSto[1]);
    QS_OBJ_DICTIONARY(l_philoQueueSto[2]);
    QS_OBJ_DICTIONARY(l_philoQueueSto[3]);
    QS_OBJ_DICTIONARY(l_philoQueueSto[4]);

    QP::QF::psInit(l_subscrSto, Q_DIM(l_subscrSto)); // init publish-subscribe

                                                  // initialize event pools...
    QP::QF::poolInit(l_smlPoolSto,
                     sizeof(l_smlPoolSto), sizeof(l_smlPoolSto[0]));

                                                // start the active objects...
    for (uint8_t n = 0U; n < N_PHILO; ++n) {
        AO_Philo[n]->start((uint8_t)(n + 1U),
                           l_philoQueueSto[n], Q_DIM(l_philoQueueSto[n]),
                           (void *)0, 0U);
    }
    AO_Table->start((uint8_t)(N_PHILO + 1U),
                    l_tableQueueSto, Q_DIM(l_tableQueueSto),
                    (void *)0, 0U);

    return QP::QF::run();                            // run the QF application
}

} // namespace DPP
