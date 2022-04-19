extern QActive *AO_Table;


Q_STATE_DEF(Philo, hungry) {
    QP::QState status_;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            TableEvt *pe = Q_NEW_X(TableEvt, 5U, /* <- margin */
                                   HUNGRY_SIG);
            if (te != nullptr) {
                pe->philoNum = PHILO_ID(this);
                AO_Table->POST_X(pe, 2U, this); // <----
            }
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
