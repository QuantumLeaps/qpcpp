void QActive::run(void) {
    do {
        QEvent const *e;
        QACTIVE_GET_(this, e);                               // wait for event
        dispatch(e);    // dispatch event to the active object's state machine
        QF::gc(e);      // check if the event is garbage, and collect it if so
    } while (m_running);

    unsubscribeAll();                          // unsubscribe from all signals
    QF::remove_(this);            // remove this object from any subscriptions
}
