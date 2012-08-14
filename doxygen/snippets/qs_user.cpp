enum UserSpyRecords {
    QS_QDPP_DISPLAY = QS_USER                      // define user record types
    . . .
};

void displyPhilStat(uint8_t n, char const *stat) {
    . . .

    QS_BEGIN(QS_QDPP_DISPLAY);                      // output a user QS record
        QS_U8(1, n);
        QS_STR(stat);
    QS_END();
}
