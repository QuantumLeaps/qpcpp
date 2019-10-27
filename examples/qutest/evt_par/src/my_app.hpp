#ifndef MY_APP_HPP
#define MY_APP_HPP

using namespace QP;

enum MySignals {
    MY_EVT0_SIG = Q_USER_SIG,
    MY_EVT1_SIG,
    MAX_PUB_SIG,    // the last published signal

    MY_EVT2_SIG,
    MY_EVT3_SIG,
    MAX_SIG         // the last signal
};

struct MyEvt1 : public QEvt {
    uint32_t u32;
};

struct MyEvt2 : public QEvt {
    uint32_t u32;
    uint16_t u16;
};

struct MyEvt3 : public QEvt {
    uint32_t u32;
    uint16_t u16;
    uint8_t  u8;
};

extern QActive * const AO_MyAO;

#endif // MY_APP_HPP
