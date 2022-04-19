
main() {
    . . .
    static Philo l_philo[N];  // N Philosopher active objects
    static QEvt const *l_philQueueSto[N][N]; // storage for Philo event queues
    static int l_philoStk[N][256]; // stacks for the Philosopher active objects
    for (std::uint8_t n = 0U; n < N; ++n) {
        uint32_t options = 0x1234U;
        ie.philNum = n;
        l_philo[n].start(
            n*10U + 1U,                 // QP priority [1..QF_MAX_ACTIVE]
            l_philoQueueSto[n],         // queue storage
            Q_DIM(l_philoQueueSto[n]),  // queue depth [QEvt* pointers]
            l_philoStk[n],              // RTOS stack storage
            sizeof(l_philoStk[n]),      // RTOS stack size [bytes]
            &options);                  // initialization parameter
    }
    . . .
}
