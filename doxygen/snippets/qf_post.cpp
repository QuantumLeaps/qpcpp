extern QActive * const AO_Table;

Q_STATE_DEF(Philo, hungry) {
    QP::QState status_;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            TableEvt *pe = Q_NEW(TableEvt, HUNGRY_SIG);
            pe->philoNum = PHILO_ID(this);
            AO_Table->POST(pe, this); // <----
            status_ = Q_RET_HANDLED;
            break;
        }
        . . .
        default: {
            status_ = super(&top);
            break;
        }
    }
    return status_;
}
