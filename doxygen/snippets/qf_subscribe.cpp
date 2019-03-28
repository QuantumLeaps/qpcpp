Q_State_DEF(Table, initial) {
    uint8_t n;
    subscribe(HUNGRY_SIG);    // subscribe to HUNGRY
    subscribe(DONE_SIG);      // subscribe to DONE
    subscribe(TERMINATE_SIG); // subscribe to TERMINATE
    for (n = 0; n < N; ++n) {
        m_fork_[n] = FREE;
        m_isHungry_[n] = 0;
    }
    return tran(&serving);
}
