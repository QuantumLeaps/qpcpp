#include "qep.h"      // QEP public interface
#include "qbomb.h"    // QBomb FSM derived from QFsm

static QBomb l_qbomb; // an instance of QBomb FSM

int main() {
    QBombInitEvt ie;

    l_qbomb.init(&ie);   // trigger initial transition

    for (;;) {           // event loop
        QEvt e;
        . . .
        // wait for the next event and assign it to the event object e
        . . .
        l_qbomb.dispatch(&e);  // dispatch the event
    }
    return 0;
}
