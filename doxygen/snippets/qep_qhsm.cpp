class Calc : public QHsm {  // derived from QHsm
private:
    double  m_operand1;
    double  m_operand2;
    char    m_display[DISP_WIDTH + 1];
    uint8_t m_len;
    uint8_t m_opKey;

public:
    Calc()             // constructor
      : QHsm(Calc)) {  // superclass' constructor
    }

protected:
    Q_STATE_DECL(initial);
    Q_STATE_DECL(on);
    Q_STATE_DECL(error);
    Q_STATE_DECL(ready);
    Q_STATE_DECL(result);
    Q_STATE_DECL(begin);
    . . .
};
