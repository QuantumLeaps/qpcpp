class Bomb : public QFsm {  // derived from QFsm
    uint8_t m_timeout;      // number of seconds till explosion
    uint8_t m_defuse;       // the secret defuse code
    uint8_t m_code;         // the current defuse code entry

public:
    Bomb() : QFsm(Q_STATE_CAST(&QBomb::initial)) {
    }

protected:
    static QState initial(Bomb * const me, QEvt const *e);
    static QState setting(Bomb * const me, QEvt const *e);
    static QState timing(Bomb * const me, QEvt const *e);
    static QState blast(Bomb * const me, QEvt const *e);
};
