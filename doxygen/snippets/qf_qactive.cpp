class Philosopher : public QActive {
private:
    uint8_t  m_num;                             // number of this philosopher
    QTimeEvt m_timeEvt;                       // to timeout thining or eating

public:
    Philosopher::Philosopher()
    : QActive((QStateHandler)&Philosopher::initial),
      m_timeEvt(TIMEOUT_SIG)
    {}

protected:
    static QState initial (Philosopher *me, QEvent const *e);
    static QState thinking(Philosopher *me, QEvent const *e);
    static QState hungry  (Philosopher *me, QEvent const *e);
    static QState eating  (Philosopher *me, QEvent const *e);
};
