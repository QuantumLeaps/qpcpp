class Philo : public QActive { // derives from QActive
private:
    uint8_t  m_num;            // number of this philosopher
    QTimeEvt m_timeEvt;        // to timeout thining or eating

public:
    Philo::Philo()
    : QActive(Q_STATE_CAST(&Philo::initial)),
      m_timeEvt(TIMEOUT_SIG, this, 0U)
    {}

protected:
    static QState initial (Philo * const me, QEvt const *e);
    static QState thinking(Philo * const me, QEvt const *e);
    static QState hungry  (Philo * const me, QEvt const *e);
    static QState eating  (Philo * const me, QEvt const *e);
};
