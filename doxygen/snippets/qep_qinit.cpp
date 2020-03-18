// initial pseudostate of the Bomb FSM ......................................
QState Bomb::initial(Bomb * const me, void const *e) {
    Q_REQUIRE(e != nullptr); // initialization event expected
    me->updateState("top-INIT");
    me->timeout_ = INIT_TIMEOUT;
    me->defuse_ = Q_EVT_CAST(BombInitEvt)->defuse;
    return Q_TRAN(&Bomb::setting);
}

// state handler function for the Calc HSM ..................................
QState Calc::on(Calc * const me, QEvt const *e) {
    switch (e->sig) {
        . . .
        case Q_INIT_SIG: {
            me->updateState("on-INIT");
            return Q_TRAN(&Calc::ready);
        }
        . . .
    }
    return Q_SUPER(&QHsm::top);
}
