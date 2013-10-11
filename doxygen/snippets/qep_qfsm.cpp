class QBomb : public QFsm { // derived from QFsm
    uint8_t m_timeout;      // number of seconds till explosion
    uint8_t m_defuse;       // the secret defuse code
    uint8_t m_code;         // the current defuse code entry

public:
    QBomb() : QFsm(Q_STATE_CAST(&QBomb::initial)) {
    }

protected:
    static QState initial(QBomb * const me, QEvt const *e);
    static QState setting(QBomb * const me, QEvt const *e);
    static QState timing(QBomb * const me, QEvt const *e);
    static QState blast(QBomb * const me, QEvt const *e);
};
