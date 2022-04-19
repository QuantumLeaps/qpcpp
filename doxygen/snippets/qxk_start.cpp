#include "qpcpp.hpp"
. . .

//............................................................................
int main() {
     . . .

    QP::QF::init();  // initialize the framework and the underlying RT kernel
    BSP::init(); // initialize the BSP
    . . .
    // start the extended threads...
    static QP::QEvt const *test1QueueSto[5];
    static uint64_t test1StackSto[64];
    DPP::XT_Test1->start( // <== start an extended thread
            1U,                      // QP prio of the thread
            test1QueueSto,           // event queue storage
            Q_DIM(test1QueueSto),    // queue length [events]
            test1StackSto,           // stack storage
            sizeof(test1StackSto));  // stack size [bytes]
    . . .
    // start active objects (basic threads)...
    static QP::QEvt const *tableQueueSto[N_PHILO];
    DPP::AO_Table->start( // <== start a basic thread (AO)
            N_PHILO + 7U,            // QP priority of the AO
            tableQueueSto,           // event queue storage
            Q_DIM(tableQueueSto),    // queue length [events]
            nullptr, 0U);            // NO stack storage

    return QP::QF::run(); // run the QF application
}
