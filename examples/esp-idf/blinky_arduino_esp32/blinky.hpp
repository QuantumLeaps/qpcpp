#ifndef BLINKY_HPP
#define BLINKY_HPP

enum BlinkySignals {
    TIMEOUT_SIG = 50,
    MAX_SIG
};

extern QP::QActive * const AO_Blinky;


#endif // BLINKY_HPP
