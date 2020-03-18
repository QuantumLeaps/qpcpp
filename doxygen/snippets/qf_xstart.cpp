namespace DPP {
    extern QP::QXThread * const XT_Test1;
} // namespace DPP


int main() {
    // stacks and queues for the extended test threads
    static QP::QEvt const *test1QueueSto[5];
    static uint64_t test1StackSto[64];

    // start the extended Test1 thread
    // start the extended Test1 thread
    DPP::XT_Test1->start(
            1U,                      // QP prio of the thread
            test1QueueSto,           // event queue storage
            Q_DIM(test1QueueSto),    // queue length [events]
            test1StackSto,           // stack storage
            sizeof(test1StackSto),   // stack size [bytes]
            nullptr);                // initialization event
    . . .
}
