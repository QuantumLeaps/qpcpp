QState Table::initial(Table *me, QEvent const *) {
    uint8_t n;
    me->subscribe(HUNGRY_SIG);                          // subscribe to HUNGRY
    me->subscribe(DONE_SIG);                              // subscribe to DONE
    me->subscribe(TERMINATE_SIG);                    // subscribe to TERMINATE
    for (n = 0; n < N; ++n) {
        me->fork_[n] = FREE;
        me->isHungry_[n] = 0;
    }
    return Q_TRAN(&Table::serving);
}
