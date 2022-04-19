class Blinky : public QP::QActive { // inherits QP::QActive
    . . .
public:
    Blinky::Blinky() // public default constructor
      : QActive(Q_STATE_CAST(&initial)),
        m_timeEvt(TIMEOUT_SIG, this, 0U)
    {}
    . . .
};
