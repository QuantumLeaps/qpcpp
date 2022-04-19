// initial pseudostate of the Blinky HSM ......................................
Q_STATE_DEF(Blinky, initial) {
    static_cast<void>(e); // unused parameter

    // arm the time event to expire in half a second and every half second
    m_timeEvt.armX(BSP_TICKS_PER_SEC/2U, BSP_TICKS_PER_SEC/2U);

    return tran(&off);
}

// state handler for the Blinky HSM ..................................
Q_STATE_DEF(Blinky, off) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            BSP_ledOff();
            status = Q_RET_HANDLED;
            break;
        }
        case TIMEOUT_SIG: {
            status = tran(&on);
            break;
        }
        default: {
            status = super(&top);
            break;
        }
    }
    return status;
}
