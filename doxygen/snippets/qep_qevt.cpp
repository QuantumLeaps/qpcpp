// example of event signals
enum MySignals {
    TIMEOUT_SIG = QP::Q_USER_SIG, // <===
    BUTTON_PRESSED,
    BUTTON_RELEASED,
    . . .
};

// example of an event with paramters
struct QCalcEvt : public QP::QEvt {  // inherits QP::QEvt
    std::uint8_t keyId;              // ID of the key depressed
};
