// state handler function for the QBomb FSM ..................................
QState QBomb::setting(QBomb *me, QEvent const *e) {
    switch (e->sig) {
        . . .
        case ARM_SIG: {
            return Q_TRAN(&QBomb::timing);
        }
    }
    return Q_IGNORED();
}

// state handler function for the QCalc HSM ..................................
QState QCalc::begin(QCalc *me, QEvent const *e) {
    switch (e->sig) {
        . . .
        case OPER_SIG: {
            if (((QCalcEvt *)e)->keyId == KEY_MINUS) {
                return Q_TRAN(&QCalc::negated1);
            }
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&QCalc::ready);
}

