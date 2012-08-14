// case 1: Interrupt Controller available,
// "unconditional interrupt unlocking" critical section policy
// (nesting of critical sections _not_ allowed)
//
interrupt void ISR_timer() {     // entered with interrupts locked in hardware
    QF_INT_UNLOCK(dummy);                                 // unlock interrupts

    QF::tick();                              //<-- call the QF tick processing

    QF_INT_LOCK(dummy);                               // lock interrupts again
    // send the EOI instruction to the Interrupt Controller
}

// case 2: Interrupt Controller not used,
// "saving and restoring interrupt status" critical section policy
// (nesting of critical sections allowed)
//
interrupt void ISR_timer() {
    QF::tick();                              //<-- call the QF tick processing
}
