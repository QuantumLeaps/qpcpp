Q_State_DEF(Table, initial) {

    subscribe(HUNGRY_SIG);    // <--- subscribe to HUNGRY
    subscribe(DONE_SIG);      // <--- subscribe to DONE
    subscribe(TERMINATE_SIG); // <--- subscribe to TERMINATE

    for (std::uint8_t n = 0U; n < N; ++n) {
        m_fork_[n] = FREE;
        m_isHungry_[n] = 0U;
    }
    return tran(&serving);
}
