    QMutex mux;
    . . .
    mux = QK::mutexLock(PRIO_CEILING);

    // access the shared resource

    QK::mutexUnlock(mux);

    . . . 