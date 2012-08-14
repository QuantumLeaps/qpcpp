QState Philosopher::eating(Philosopher *me, QEvent const *e) {
    TableEvt *pe;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            me->timeEvt_.postIn(me, EAT_TIME);      // arm one-shot time event
            return Q_HANDLED();
        }
        case TIMEOUT_SIG: {
            return Q_TRAN(&Philosopher::thinking);
        }
        case Q_EXIT_SIG: {
            busyDelay();
            pe = Q_NEW(TableEvt, DONE_SIG);
            pe->philNum = me->m_num;
            QF::publish(pe);
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&QHsm::top);
}
