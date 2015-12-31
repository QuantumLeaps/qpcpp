#include "qep.h"    // QEP public interface
#include "bomb.h"   // Bomb FSM derived from QFsm

static Bomb l_bomb; // an instance of QBomb FSM

int main() {
    BombInitEvt ie;

    l_qbomb.init(&ie);   // trigger initial transition

    for (;;) {           // event loop
        QEvt e;
        . . .
        // wait for the next event and assign it to the event object e
        . . .
        l_bomb.dispatch(&e);  // dispatch the event
    }
    return 0;
}
