Philo Philo::inst;
. . .
for (std::uint8_t n = 0U; n < N_PHILO; ++n) {
    QS_OBJ_ARR_DICTIONARY(&Philo::inst[n], n);
    QS_OBJ_ARR_DICTIONARY(&Philo::inst[n].m_timeEvt, n);
}
