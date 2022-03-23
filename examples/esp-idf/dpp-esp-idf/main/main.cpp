#include "esp_log.h"
#include "qpcpp.hpp"
#include "dpp.hpp"
#include "freertos/FreeRTOS.h"

using namespace QP;
static constexpr unsigned stack_size = 2048;

Q_DEFINE_THIS_FILE
static const char * TAG = "main";

extern "C" {
	void app_main();
}


void app_main()
{
    QF::init(); // initialize the framework

    // init publish-subscribe
    static QSubscrList subscrSto[MAX_PUB_SIG];
    QF::psInit(subscrSto, Q_DIM(subscrSto));

    // initialize event pools...
    static QF_MPOOL_EL(TableEvt) smlPoolSto[2*N_PHILO];
    QF::poolInit(smlPoolSto,
                 sizeof(smlPoolSto), sizeof(smlPoolSto[0]));
    // start Philos
    static QP::QEvt const *philoQueueSto[10][N_PHILO];
    for (uint8_t n = 0U; n < N_PHILO; ++n) {
		AO_Philo[n]->setAttr(FreeRTOS_TaskAttrs::TASK_NAME_ATTR, "Philo");
        AO_Philo[n]->start((uint_fast8_t)(n + 1U), // priority
                            philoQueueSto[n],
                            Q_DIM(philoQueueSto[n]),
                            nullptr, 
                            stack_size);
    }
    // start Table
    static QP::QEvt const *tableQueueSto[N_PHILO];
	AO_Table->setAttr(QP::FreeRTOS_TaskAttrs::TASK_NAME_ATTR, "Table");
    AO_Table->start((uint_fast8_t)(N_PHILO + 1U), // priority
                    tableQueueSto,
                    Q_DIM(tableQueueSto),
                    nullptr,
                    stack_size);
    QF::run(); // run the QF/C++ framework
}
