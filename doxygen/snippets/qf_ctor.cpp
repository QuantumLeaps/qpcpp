class Philo : public QActive { // derives from QActive
    . . .
public:
    Philo::Philo() // public default constructor
    : QActive(Q_STATE_CAST(&Philosopher::initial)),
      m_timeEvt(TIMEOUT_SIG, this, 0U)
    {}
    . . .
};
