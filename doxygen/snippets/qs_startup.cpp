#ifdef Q_SPY

bool QS::onStartup(void const *arg) {
    static uint8_t qsTxBuf[1024]; // buffer for QS transmit channel
    static uint8_t qsRxBuf[100];  // buffer for QS receive channel

    initBuf(qsTxBuf, sizeof(qsTxBuf));
    rxInitBuf(qsRxBuf, sizeof(qsRxBuf));

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
