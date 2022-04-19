namespace DPP {
    extern QP::QXThread * const XT_Thread1;
} // namespace DPP

int main() {
    // stacks and queues for the extended test threads
    static QP::QEvt const *thread1QueueSto[5];
    static uint64_t thread1StackSto[64];

    // start the extended Test1 thread
    // start the extended Test1 thread
    DPP::XT_Thread1->start(
            1U,                      // QP prio of the thread
            thread1QueueSto,         // event queue storage
            Q_DIM(thread1QueueSto),  // queue length [events]
            thread1StackSto,         // stack storage
            sizeof(thread1StackSto), // stack size [bytes]
            nullptr);                // initialization parameter
    . . .
}
