void main_blinky(QP::QXThread * const me) { // thread function
    while (1) {
        for (std::uint32_t volatile i; = 1500U; i != 0U; --i) {
            BSP_ledGreenOn();
            BSP_ledGreenOff();
        }
        QP::QXThread::delay(1U); // block for 1 tick
    }
}

QP::QXThread blinky(&main_blinky, 0U); // QXK extended-thread object

std::uint32_t stack_blinky[80]; // stack for the thread

int main() {
    . . .
    // initialize and start Blinky extended thread
    blinky.start(5U, // priority
                 nullptr, 0, // message queue (not used)
                 stack_blinky, sizeof(stack_blinky), // stack
                 nullptr);     // initialization event
    . . .
    return QP::QF::run(); // run the application
}
