class Calc : public QMsm {  // derived from QMsm
private:
    double  m_operand;
    char    m_display[DISP_WIDTH + 1];
    uint8_t m_len;
    uint8_t m_opKey;

public:
    Calc() : QMsm(Q_STATE_CAST(&Calc::initial)) {  // ctor
    }

protected:
    // NOTE: QMsm state machine code is not intended for manual
    // coding but rather needs to be generated automatically by
    // the QM modeling tool
    static QState initial  (Calc * const me, QEvt const *e);
    static QState on       (Calc * const me, QEvt const *e);
    static QState error    (Calc * const me, QEvt const *e);
    static QState ready    (Calc * const me, QEvt const *e);
    static QState result   (Calc * const me, QEvt const *e);
    static QState begin    (Calc * const me, QEvt const *e);
    . . .
};
