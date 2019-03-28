class Philo : public QActive { // derives from QActive
private:
    uint8_t  m_num;            // number of this philosopher
    QTimeEvt m_timeEvt;        // to timeout thining or eating

public:
    Philo::Philo()             // constructor
    : QActive(&initial),       // superclass' constructor
      m_timeEvt(TIMEOUT_SIG, this, 0U)
    {}

protected:
    Q_STATE_DECL(initial);
    Q_STATE_DECL(thinking);
    Q_STATE_DECL(hungry);
    Q_STATE_DECL(eating);
};
