QState Table::initial(Table *me, QEvent const *) {
    uint8_t n;

    QS_SIG_DICTIONARY(HUNGRY_SIG, me);   // output signal dictionary QS record
    QS_SIG_DICTIONARY(DONE_SIG, me);     // output signal dictionary QS record
    QS_SIG_DICTIONARY(EAT_SIG, 0);       // output signal dictionary QS record

    QS_FUN_DICTIONARY(Table::serving);

    subscribe(HUNGRY_SIG);
    subscribe(DONE_SIG);
    subscribe(TERMINATE_SIG);

    for (n = 0; n < N; ++n) {
        me->fork__[n] = FREE;
        me->isHungry__[n] = 0;
    }
    return Q_TRAN(&Table::serving);
}
