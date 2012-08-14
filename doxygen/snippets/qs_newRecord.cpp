#ifdef Q_SPY

void QS::newRecord(void) {
    uint32_t nBytes = 256;
    uint8_t const *block;
    while ((block = getBlock(&nBytes)) != (uint8_t *)0) {
        fwrite(block, 1, nBytes, l_qsFile);
        nBytes = 256;
    }
}

. . .

#endif // Q_SPY
