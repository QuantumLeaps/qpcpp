#include "qep.hpp"  // QEP public interface
#include "calc.h"   // Calc HSM derived from QHsm

static Calc l_calc; // an instance of Calc HSM

int main() {

    l_calc.init(0U); // trigger initial transition

    for (;;) {       // event loop
        QEvt e;
        . . .
        // wait for the next event and assign it to the event object e
        . . .
        l_calc.dispatch(&e, 0U); // dispatch the event
    }
    return 0;
}
