class QBomb : public QFsm {
    uint8_t m_timeout;                     // number of seconds till explosion
    uint8_t m_defuse;                                // the secret defuse code
    uint8_t m_code;                           // the current defuse code entry

public:
    QBomb() : QFsm((QStateHandler)&QBomb::initial) {
    }

protected:
    static QState initial(QBomb *me, QEvent const *e);
    static QState setting(QBomb *me, QEvent const *e);
    static QState timing(QBomb *me, QEvent const *e);
    static QState blast(QBomb *me, QEvent const *e);
};
