namespace DPP {
. . .
QP::QState Philo::eating(Philo * const me, QP::QEvt const * const e) {
    QP::QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            me->m_timeEvt.postIn(me, eat_time());
            status = Q_HANDLED();
            break;
        }
        case Q_EXIT_SIG: {
            TableEvt *pe = Q_NEW(TableEvt, DONE_SIG);
            pe->philoNum = PHILO_ID(me);
            QP::QF::PUBLISH(pe, me);
            (void)me->m_timeEvt.disarm();
            status = Q_HANDLED();
            break;
        }
        case TIMEOUT_SIG: {
            status = Q_TRAN(&Philo::thinking);
            break;
        }
        case EAT_SIG: // intentionally fall through
        case DONE_SIG: {
            // EAT or DONE must be for other Philos than this one
            Q_ASSERT(Q_EVT_CAST(TableEvt)->philoNum != PHILO_ID(me));
            status = Q_HANDLED();
            break;
        }
        default: {
            status = Q_SUPER(&QHsm::top);
            break;
        }
    }
    return status;
}

} // namespace DPP

