extern QActive *AO_Table;

QP::QState Philo::hungry(Philo * const me, QP::QEvt const * const e) {
    QP::QState status_;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            TableEvt *te;
            Q_NEW_X(te, TableEvt, 1U, HUNGRY_SIG, PHILO_ID(me));
            if (te != static_cast<TableEvt *>(0)) {
                AO_Table->POST_X(te, 1U, me);
            }
            status_ = Q_HANDLED();
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
