/*
* Copyright (C) 2022 Victor Chavez
* This program is open source software: you can redistribute it and/or
* modify it under the terms of the GNU General Public License as published
* by the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* Alternatively, this program may be distributed and modified under the
* terms of Quantum Leaps commercial licenses, which expressly supersede
* the GNU General Public License and are specifically designed for
* licensees interested in retaining the proprietary status of their code.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <www.gnu.org/licenses>.
*/
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
