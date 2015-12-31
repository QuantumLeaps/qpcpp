class Calc : public QHsm {  // derived from QHsm
private:
    double  m_operand1;
    double  m_operand2;
    char    m_display[DISP_WIDTH + 1];
    uint8_t m_len;
    uint8_t m_opKey;

public:
    Calc() : QHsm(Q_STATE_CAST(&QCalc::initial)) {  // ctor
    }

protected:
    static QState initial  (Calc * const me, QEvt const *e);
    static QState on       (Calc * const me, QEvt const *e);
    static QState error    (Calc * const me, QEvt const *e);
    static QState ready    (Calc * const me, QEvt const *e);
    static QState result   (Calc * const me, QEvt const *e);
    static QState begin    (Calc * const me, QEvt const *e);
    . . .
};
