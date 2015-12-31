// state handler function for the Bomb FSM ..................................
QState Bomb::setting(Bomb * const me, QEvt const *e) {
    switch (e->sig) {
        . . .
        case ARM_SIG: {
            return Q_TRAN(&Bomb::timing);
        }
    }
    return Q_IGNORED();
}

// state handler function for the Calc HSM ..................................
QState Calc::begin(Calc * const me, QEvt const *e) {
    switch (e->sig) {
        . . .
        case OPER_SIG: {
            if (((CalcEvt *)e)->keyId == KEY_MINUS) {
                return Q_TRAN(&Calc::negated1);
            }
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&Calc::ready);
}

