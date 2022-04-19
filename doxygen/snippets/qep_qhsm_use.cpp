#include "qep.hpp"  // QEP public interface
#include "calc.h"   // Calc HSM derived from QHsm

Calc Calc::inst;    // a single instance of Calc HSM

int main() {

    Calc::inst.init(0U); // trigger initial transition

    for (;;) {       // event loop
        QP::QEvt e;
        . . .
        // wait for the next event and assign it to the event object e
        . . .
        Calc::inst.dispatch(&e, 0U); // dispatch the event
    }
    return 0;
}
