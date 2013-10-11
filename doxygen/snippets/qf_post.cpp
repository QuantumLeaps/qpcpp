extern QActive *AO_Table;

QP::QState Philo::hungry(Philo * const me, QP::QEvt const * const e) {
    QP::QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            TableEvt *pe = Q_NEW(TableEvt, HUNGRY_SIG);
            pe->philoNum = PHILO_ID(me);
            AO_Table->POST(pe, me);
            status = Q_HANDLED();
            break;
        }
        . . .
        default: {
            status = Q_SUPER(&QHsm::top);
            break;
        }
    }
    return status;
}
