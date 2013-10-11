static Table l_table;

QP::QState Table::initial(Table * const me, QP::QEvt const * const e) {
    (void)e; // suppress the compiler warning about unused parameter

    QS_OBJ_DICTIONARY(&l_table);
    QS_FUN_DICTIONARY(&QP::QHsm::top);
    QS_FUN_DICTIONARY(&Table::initial);
    QS_FUN_DICTIONARY(&Table::active);
    QS_FUN_DICTIONARY(&Table::serving);
    QS_FUN_DICTIONARY(&Table::paused);

    QS_SIG_DICTIONARY(DONE_SIG,      (void *)0); // global signals
    QS_SIG_DICTIONARY(EAT_SIG,       (void *)0);
    QS_SIG_DICTIONARY(PAUSE_SIG,     (void *)0);
    QS_SIG_DICTIONARY(TERMINATE_SIG, (void *)0);

    QS_SIG_DICTIONARY(HUNGRY_SIG,    me); // signal just for Table

    . . .
}
