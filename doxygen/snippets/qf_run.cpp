void QActive::run(void) { // <---
    m_running = true;
    while (m_running) {
        QEvt const *e = get_(e); // wait for an event
        dispatch(e, m_prio); // dispatch e to the AO's state machine
        QF::gc(e);  // check if the event is garbage, and collect it if so
    }

    unsubscribeAll();   // unsubscribe from all signals
    unregister_();  // remove this object from any subscriptions
}
