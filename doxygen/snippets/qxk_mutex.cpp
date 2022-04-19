QXMutex l_rndMutex;  // mutex to protect the random number generator


void BSP::randomSeed(uint32_t seed) {
    l_rndMutex.init(N_PHILO + 1); // <--- initialize the mutex
    l_rnd = seed;
}

uint32_t BSP::random(void) { // a very cheap pseudo-random-number generator
    uint32_t rnd;

    l_rndMutex.lock();   // <--- lock the shared random seed
    // "Super-Duper" Linear Congruential Generator (LCG)
    rnd = l_rnd * (3U*7U*11U*13U*23U);
    l_rnd = rnd; // set for the next time
    l_rndMutex.unlock(); // <--- unlock the shared random seed

    return rnd;
}
