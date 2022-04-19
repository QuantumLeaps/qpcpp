class Philo : public QP::QMActive { // inherits QP::QMActive
private:
    QTimeEvt m_timeEvt;         // to timeout thining or eating
    std::uint8_t m_num;         // number of this philosopher

public:
    Philo::Philo()              // constructor
      : QMActive(&initial),     // superclass' constructor
        m_timeEvt(TIMEOUT_SIG, this, 0U)
    {}

protected:
    // NOTE: QMsm state machine code is not intended for manual
    // coding but rather needs to be generated automatically by
    // the QM modeling tool
    QM_STATE_DECL(initial);
    QM_STATE_DECL(thinking);
    QM_STATE_DECL(hungry);
    QM_STATE_DECL(eating);
};
