// bsp.hpp -----------------
extern QP::QTicker ticker0; // "ticker" AO for clock rate 0
extern QP::QTicker ticker1; // "ticker" AO for clock rate 1

#ifdef Q_SPY
    // QS-IDs for local filtering
    static QSpyId const l_SysTick_Handler    = { QS_AP_ID + 5 };
    static QSpyId const l_Timer0A_IRQHandler = { QS_AP_ID + 6 };
#endif

// bsp.cpp -----------------
QP::QTicker ticker0; // "ticker" AO for clock rate 0
QP::QTicker ticker1; // "ticker" AO for clock rate 1

// system clock tick ISR for clock rate 0
extern "C" void SysTick_Handler(void) {
    . . .
    ticker0.POST(nullptr, &l_SysTick_Handler);
    . . .
}

// system clock tick ISR for clock rate 1
extern "C" void Timer0A_IRQHandler(void) {
    . . .
    ticker1.POST(nullptr, &l_Timer0A_IRQHandler);
    . . .
}

// main.cpp ---------------
main () {
    . . .
    ticker0.start(1U, // priority
                  nullptr, 0U, nullptr, 0U); // not used

    ticker1.start(2U, // priority
                  nullptr, 0U, nullptr, 0U); // not used
    . . .
}
