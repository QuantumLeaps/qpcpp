int main() {
    static QP::QEvt const *tableQueueSto[N_PHILO];
    static uint64_t tableStackSto[64];
    static QP::QEvt const *philoQueueSto[N_PHILO][N_PHILO];
    static uint64_t philoStackSto[N_PHILO][64];

    static QP::QSubscrList subscrSto[DPP::MAX_PUB_SIG];
    static QF_MPOOL_EL(DPP::TableEvt) smlPoolSto[2*N_PHILO]; // small pool

    // stack for the "naked" test thread
    static QP::QEvt const *testQueueSto[5];
    static uint64_t testStackSto[64];

    // stack for the QXK's idle thread
    static uint64_t idleStackSto[32];


    QP::QF::init();  // initialize the framework and the underlying RT kernel
    QP::QXK::init(idleStackSto, sizeof(idleStackSto)); // initialize QXK
    DPP::BSP::init(); // initialize the BSP

    // object dictionaries...
    . . .

    QP::QF::psInit(subscrSto, Q_DIM(subscrSto)); // init publish-subscribe

    // initialize event pools...
    QP::QF::poolInit(smlPoolSto,
                     sizeof(smlPoolSto), sizeof(smlPoolSto[0]));

    // start the active objects (basic-threads)...
    for (uint8_t n = 0U; n < N_PHILO; ++n) {
        DPP::AO_Philo[n]->start(
            static_cast<std::uint_fast8_t>(n + 1U), // QP priority of the AO
            philoQueueSto[n],          // event queue storage
            Q_DIM(philoQueueSto[n]),   // queue length [events]
            philoStackSto[n],          // stack storage
            sizeof(philoStackSto[n]),  // stack size [bytes]
            nullptr);                  // initialization parameter
    }
    . . .
    // start the extended-threads...
    DPP::XT_Test->start(
            10U,                       // QP priority of the AO
            testQueueSto,              // event queue storage
            Q_DIM(testQueueSto),       // queue length [events]
            testStackSto,              // stack storage
            sizeof(testStackSto),      // stack size [bytes]
            nullptr);                  // initialization parameter
    . . .
    return QP::QF::run(); // run the QF application
}
