QXSemaphore BTN_sema; // semaphore to signal a button press

int main() {
    . . .
    // initialize the BTN_sema semaphore as binary, signaling semaphore
    BTN_sema.init(0U,  // initial semaphore count (singaling semaphore)
                  1U); // maximum semaphore count (binary semaphore)
    . . .
}

void main_threadXYZ(QXThread * const me) {
    while (1) {
        . . .
        BTN_sema.wait(QXTHREAD_NO_TIMEOUT); // timeout for waiting
        . . .
    }
}

void GPIO_Handler(void) {
    . . .
    BTN_sema.signal();
    . . .
}
