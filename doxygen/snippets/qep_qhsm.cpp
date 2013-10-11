class QCalc : public QHsm {  // derived from QHsm
private:
    double  m_operand1;
    double  m_operand2;
    char    m_display[DISP_WIDTH + 1];
    uint8_t m_len;
    uint8_t m_opKey;

public:
    QCalc() : QHsm(Q_STATE_CAST(&QCalc::initial)) {  // ctor
    }

protected:
    static QState initial  (QCalc * const me, QEvt const *e);
    static QState on       (QCalc * const me, QEvt const *e);
    static QState error    (QCalc * const me, QEvt const *e);
    static QState ready    (QCalc * const me, QEvt const *e);
    static QState result   (QCalc * const me, QEvt const *e);
    static QState begin    (QCalc * const me, QEvt const *e);
    . . .
};
