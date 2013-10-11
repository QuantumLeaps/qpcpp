#ifdef Q_SPY

bool QS::onStartup(void const *arg) {
    static uint8_t qsBuf[4*1024];  // 4K buffer for Quantum Spy
    initBuf(qsBuf, sizeof(qsBuf));

    // configure UART 0/1 for QSPY output ...
    if (*(char const *)arg == '0') { // use UART 0
           // configure UART 0 for QSPY output ...
        . . .
        return true;  // UART 0 successfully opened
    }
    else {                           // use UART 1
        // configure UART 1 for QSPY output ...
        . . .
        return false; // UART 1 successfully opened
    }
}

. . .

#endif // Q_SPY
