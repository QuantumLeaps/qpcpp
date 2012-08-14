extern QActive *AO_Table;

QState Philosopher::hungry(Philosopher *me, QEvent const *e) {
    TableEvt *pe;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            pe = Q_NEW(TableEvt, HUNGRY_SIG);    // dynamically allocate event
            pe->philNum = me->num_;
            AO_Table->postFIFO(pe);               // post the event directly
            return Q_HANDLED();
        }
        . . .
    }
    return Q_SUPER(&QHsm::top);
}
