#ifndef BLINKY_HPP
#define BLINKY_HPP

enum BlinkySignals {
    TIMEOUT_SIG = QP::Q_USER_SIG,
    MAX_SIG
};

extern QP::QActive * const AO_Blinky;


#endif // BLINKY_HPP
