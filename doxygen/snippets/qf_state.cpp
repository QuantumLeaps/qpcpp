namespace DPP {
. . .
Q_STATE_DEF(Philo, eating) {
    QP::QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            m_timeEvt.armX(eat_time());
            status = Q_RET_HANDLED;
            break;
        }
        case Q_EXIT_SIG: {
            TableEvt *pe = Q_NEW(TableEvt, DONE_SIG);
            pe->philoNum = PHILO_ID(this);
            QP::QF::PUBLISH(pe, me);
            m_timeEvt.disarm();
            status = Q_RET_HANDLED);
            break;
        }
        case TIMEOUT_SIG: {
            status = tran(&thinking);
            break;
        }
        case EAT_SIG: // intentionally fall through
        case DONE_SIG: {
            // EAT or DONE must be for other Philos than this one
            Q_ASSERT(Q_EVT_CAST(TableEvt)->philoNum != PHILO_ID(this));
            status = Q_RET_HANDLED;
            break;
        }
        default: {
            status = super(&top);
            break;
        }
    }
    return status;
}

} // namespace DPP

