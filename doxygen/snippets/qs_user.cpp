#ifdef Q_SPY
    enum {
        PHILO_STAT = QP::QS_USER
    };
    . . .
#endif

void BSP_displayPhilStat(uint8_t n, char const *stat) {
    . . .

    QS_BEGIN(PHILO_STAT, AO_Philo[n]) // application-specific record begin
        QS_U8(1, n);                  // Philosopher number
        QS_STR(stat);                 // Philosopher status
    QS_END()
}
