std::uint32_t BSP::random(void) {

    // lock the scheduler around l_rnd up to the (N_PHILO + 1U) ceiling
    QP::QSchedStatus lockStat = QP::QXK::schedLock(N_PHILO + 1U);

    // "Super-Duper" Linear Congruential Generator (LCG)
    // LCG(2^32, 3*7*11*13*23, 0, seed)
    //
    uint32_t rnd = l_rnd * (3U*7U*11U*13U*23U);
    l_rnd = rnd; // set for the next time

    QP::QXK::schedUnlock(lockStat); // unlock sched after accessing l_rnd

    return (rnd >> 8);
}

