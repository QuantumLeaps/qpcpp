// initial pseudostate of the QBomb FSM ......................................
QState QBomb::initial(QBomb *me, QEvent const *e) {
    Q_REQUIRE(e != (QEvent const *)0);        // initialization event expected
    me->updateState("top-INIT");
    me->timeout_ = INIT_TIMEOUT;
    me->defuse_ = ((QBombInitEvt const *)e)->defuse;
    return Q_TRAN(&QBomb::setting);
}

// state handler function for the QCalc HSM ..................................
QState QCalc::on(QCalc *me, QEvent const *e) {
    switch (e->sig) {
        . . .
        case Q_INIT_SIG: {
            me->updateState("on-INIT");
            return Q_TRAN(&QCalc::ready);
        }
        . . .
    }
    return Q_SUPER(&QHsm::top);
}
