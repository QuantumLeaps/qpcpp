// state handler function for the Blinky HSM ..................................
Q_STATE_DEF(Blinky, off) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            BSP_ledOff();
            status = Q_RET_HANDLED;
            break;
        }
        case TIMEOUT_SIG: {
            status = tran(&on); // <--- transition
            break;
        }
        default: {
            status = super(&top);
            break;
        }
    }
    return status;
}
