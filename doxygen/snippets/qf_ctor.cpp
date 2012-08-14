class Philosopher : public QActive {
    . . .
public:
    Philosopher::Philosopher()                   // public default constructor
    : QActive((QStateHandler)&Philosopher::initial),
      m_timeEvt(TIMEOUT_SIG)
    {}
    . . .
};
