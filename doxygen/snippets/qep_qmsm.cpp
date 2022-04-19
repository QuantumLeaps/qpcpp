class Calc : public QP::QMsm {  // inherits QP::QMsm
private:
    double  m_operand;
    char    m_display[DISP_WIDTH + 1];
    std::uint8_t m_len;
    std::uint8_t m_opKey;

public:
    Calc()                 // constructor
      : QMsm(&initial)) {  // superclass' constructor
    }

protected:
    // NOTE: QMsm state machine code is not intended for manual
    // coding but rather needs to be generated automatically by
    // the QM modeling tool
    QM_STATE_DECL(initial);
    QM_STATE_DECL(on);
    QM_ACTION_DECL(on_e);
    QM_STATE_DECL(ready);
    QM_STATE_DECL(result);
    QM_STATE_DECL(begin);
    . . .
};
