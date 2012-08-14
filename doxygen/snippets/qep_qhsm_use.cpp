#include "qep.h"                                     // QEP/C public interface
#include "qcalc.h"                              // QCalc HSM derived from QHsm

static QCalc l_qcalc;                              // an instance of QCalc HSM

int main() {

    l_qcalc.init();                              // trigger initial transition

    for (;;) {                                                   // event loop
        QEvent e;
        . . .
        // wait for the next event and assign it to the event object e
        . . .
        l_qcalc.dispatch(&e);                            // dispatch the event
    }
    return 0;
}
