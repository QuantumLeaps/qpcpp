#include "qpcpp.hpp"
#include "my_app.hpp"

Q_DEFINE_THIS_FILE

// MyAO declaration ----------------------------------------------------------
class MyAO : public QActive {
public:
    MyAO() : QActive(Q_STATE_CAST(&MyAO::initial)) {}
private:
    static QState initial(MyAO * const me, QEvt const * const e);
    static QState active(MyAO * const me, QEvt const * const e);
};

// Local objects -------------------------------------------------------------
static MyAO l_MyAO; // the single instance of the MyAO active object

// Global-scope objects ------------------------------------------------------
QActive * const AO_MyAO = &l_MyAO; /* "opaque" AO pointer */

// MyAO::SM ------------------------------------------------------------------
QState MyAO::initial(MyAO * const me, QEvt const * const e) {
    (void)e; /* unused parameter */

    QS_FUN_DICTIONARY(&QHsm::top);
    QS_FUN_DICTIONARY(&MyAO::initial);
    QS_FUN_DICTIONARY(&MyAO::active);

    QS_SIG_DICTIONARY(MY_EVT0_SIG,   nullptr);
    QS_SIG_DICTIONARY(MY_EVT1_SIG,   nullptr);
    QS_SIG_DICTIONARY(MY_EVT2_SIG,   nullptr);
    QS_SIG_DICTIONARY(MY_EVT3_SIG,   nullptr);

    return Q_TRAN(&MyAO::active);
}
//${AOs::MyAO::SM::active} ...................................................
QState MyAO::active(MyAO * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        case MY_EVT0_SIG: {
            status_ = Q_HANDLED();
            break;
        }
        case MY_EVT1_SIG: {
            status_ = Q_HANDLED();
            break;
        }
        case MY_EVT2_SIG: {
            status_ = Q_HANDLED();
            break;
        }
        case MY_EVT3_SIG: {
            status_ = Q_HANDLED();
            break;
        }
        default: {
            status_ = Q_SUPER(&QHsm::top);
            break;
        }
    }
    return status_;
}

