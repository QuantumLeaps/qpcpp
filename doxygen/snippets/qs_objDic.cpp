Table Table::inst;

Q_STATE_DEF(Table, initial) {

    QS_OBJ_DICTIONARY(&Table::inst);
    QS_FUN_DICTIONARY(&Table::initial);
    QS_FUN_DICTIONARY(&Table::active);
    QS_FUN_DICTIONARY(&Table::serving);
    QS_FUN_DICTIONARY(&Table::paused);

    QS_SIG_DICTIONARY(DONE_SIG,      nullptr); // global signals
    QS_SIG_DICTIONARY(EAT_SIG,       nullptr);
    QS_SIG_DICTIONARY(PAUSE_SIG,     nullptr);
    QS_SIG_DICTIONARY(TERMINATE_SIG, nullptr);

    QS_SIG_DICTIONARY(HUNGRY_SIG,    this); // signal just for Table

    . . .
}
