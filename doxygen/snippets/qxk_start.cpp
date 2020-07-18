#include "qpcpp.hpp"
. . .

//............................................................................
int main() {
    static QP::QEvt const *tableQueueSto[N_PHILO];
    static QP::QEvt const *philoQueueSto[N_PHILO][N_PHILO];
    . . .
    // stacks and queues for the extended test threads
    static QP::QEvt const *test1QueueSto[5];
    static uint64_t test1StackSto[64];
    static QP::QEvt const *test2QueueSto[5];
    static uint64_t test2StackSto[64];

    QP::QF::init();  // initialize the framework and the underlying RT kernel
    BSP::init(); // initialize the BSP
    . . .
    // start the extended Test1 thread
    DPP::XT_Test1->start( // <== start an extended thread
            1U,                      // QP prio of the thread
            test1QueueSto,           // event queue storage
            Q_DIM(test1QueueSto),    // queue length [events]
            test1StackSto,           // stack storage
            sizeof(test1StackSto));  // stack size [bytes]
    . . .

    DPP::AO_Table->start( // <== start a basic thread (AO)
            N_PHILO + 7U,            // QP priority of the AO
            tableQueueSto,           // event queue storage
            Q_DIM(tableQueueSto),    // queue length [events]
            nullptr, 0U);            // no stack storage

    return QP::QF::run(); // run the QF application
}
