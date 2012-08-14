class QCalc : public QHsm {                // Quantum Calculator state machine
private:
    double  m_operand1;
    double  m_operand2;
    char    m_display[DISP_WIDTH + 1];
    uint8_t m_len;
    uint8_t m_opKey;

public:
    QCalc() : QHsm((QStateHandler)&QCalc::initial) {                   // ctor
    }

protected:
    static QState initial  (QCalc *me, QEvent const *e);
    static QState on       (QCalc *me, QEvent const *e);
    static QState error    (QCalc *me, QEvent const *e);
    static QState ready    (QCalc *me, QEvent const *e);
    static QState result   (QCalc *me, QEvent const *e);
    static QState begin    (QCalc *me, QEvent const *e);
    static QState negated1 (QCalc *me, QEvent const *e);
    static QState operand1 (QCalc *me, QEvent const *e);
    static QState zero1    (QCalc *me, QEvent const *e);
    static QState int1     (QCalc *me, QEvent const *e);
    static QState frac1    (QCalc *me, QEvent const *e);
    static QState opEntered(QCalc *me, QEvent const *e);
    static QState negated2 (QCalc *me, QEvent const *e);
    static QState operand2 (QCalc *me, QEvent const *e);
    static QState zero2    (QCalc *me, QEvent const *e);
    static QState int2     (QCalc *me, QEvent const *e);
    static QState frac2    (QCalc *me, QEvent const *e);
};
