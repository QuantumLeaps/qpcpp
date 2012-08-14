static Philosopher   l_philo[N];               // N Philosopher active objects
static QEvent const *l_philQueueSto[N][N];   // storage for Philo event queues
static int l_philoStk[N][256];    // stacks for the Philosopher active objects

main() {
    . . .
    for (n = 0; n < N; ++n) {
        TableEvt ie;           // initialization event for the Philosopher HSM
        ie.philNum = n;
        l_philo[n].start((uint8_t)(n*10 + 1),                      // priority
                      l_philoQueueSto[n], Q_DIM(l_philoQueueSto[n]),  // queue
                      l_philoStk[n], sizeof(l_philoStk[n]),  // uC/OS-II stack
                      &ie);                            // initialization event
    }
    . . .
}
