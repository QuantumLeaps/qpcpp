#include "qep.h"                                     // QEP/C public interface
#include "qbomb.h"                              // QBomb FSM derived from QFsm

static QBomb l_qbomb;                              // an instance of QBomb FSM

int main() {
    QBombInitEvt ie;
    ie.defuse = 0x0D;                                           // 1101 binary
    l_qbomb.init(&ie);                           // trigger initial transition

    for (;;) {                                                   // event loop
        QEvent e;
        . . .
        // wait for the next event and assign it to the event object e
        . . .
        l_qbomb.dispatch(&e);                            // dispatch the event
    }
    return 0;
}
