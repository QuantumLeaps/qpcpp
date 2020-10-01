#ifdef Q_SPY
    enum {
        PHILO_STAT = QP::QS_USER
    };
    . . .
#endif

void BSP_displayPhilStat(uint8_t n, char const *stat) {
    . . .

    // application-specific record
    QS_BEGIN_ID(PHILO_STAT, AO_Philo[n]->m_prio)
        QS_U8(1, n);  // Philosopher number
        QS_STR(stat); // Philosopher status
    QS_END()
}
