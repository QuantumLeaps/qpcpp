#include "qpcpp.hpp"   // QP-C++ framework
#include "blinky.hpp"  // Blinky application interface
#include "bsp.hpp"     // Board Support Package (BSP)
#include "Arduino.h"
#include "esp_freertos_hooks.h"

using namespace QP;

// QS facilities

// un-comment if QS instrumentation needed
//#define QS_ON
// BSP functions
static void freertos_tick_hook(void); /*Tick hook for QP */
static uint8_t const l_TickHook = static_cast<uint8_t>(0);
static void freertos_tick_hook(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    /* process time events for rate 0 */
    QF::TICK_FROM_ISR(&xHigherPriorityTaskWoken, &l_TickHook);
    /* notify FreeRTOS to perform context switch from ISR, if needed */
    if(xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
}



//............................................................................
void BSP::init(void) {
    // initialize the hardware used in this sketch...
    // NOTE: interrupts are configured and started later in QF::onStartup()
    pinMode(LED_BUILTIN, OUTPUT);

#ifdef QS_ON
    QS_INIT(nullptr);
    // setup the QS filters...
    QS_GLB_FILTER(QP::QS_SM_RECORDS); // state machine records
    QS_GLB_FILTER(QP::QS_AO_RECORDS); // active object records
    QS_GLB_FILTER(QP::QS_UA_RECORDS); // all user records
#endif
}
//............................................................................
void BSP::ledOff(void) {
    digitalWrite(LED_BUILTIN, LOW);
}
//............................................................................
void BSP::ledOn(void) {
    digitalWrite(LED_BUILTIN, HIGH);
}

//............................................................................
bool IdleHook(void) {
    QF_INT_ENABLE(); // simply re-enable interrupts

#ifdef QS_ON
    // transmit QS outgoing data (QS-TX)
    uint16_t len = Serial.availableForWrite();
    if (len > 0U) { // any space available in the output buffer?
        uint8_t const *buf = QS::getBlock(&len);
        if (buf) {
            Serial.write(buf, len); // asynchronous and non-blocking
        }
    }

    // receive QS incoming data (QS-RX)
    len = Serial.available();
    if (len > 0U) {
        do {
            QP::QS::rxPut(Serial.read());
        } while (--len > 0U);
        QS::rxParse();
    }
#endif // QS_ON
  return false;
}
void QF::onStartup(void) {
    esp_register_freertos_idle_hook(IdleHook);
    esp_register_freertos_tick_hook_for_cpu(freertos_tick_hook, QP_CPU_NUM);
    QS_OBJ_DICTIONARY(&l_TickHook);
}
//............................................................................
extern "C" Q_NORETURN Q_onAssert(char const * const module, int location) {
    //
    // NOTE: add here your application-specific error handling
    //
    (void)module;
    (void)location;
    Serial.print("QP Assert module:");
    Serial.print(module);
    Serial.print(",");
    Serial.println(location);
    QF_INT_DISABLE(); // disable all interrupts
    for (;;) { // sit in an endless loop for now
    }
}

//----------------------------------------------------------------------------
// QS callbacks...
#ifdef QS_ON

//............................................................................
bool QP::QS::onStartup(void const * arg) {
    static uint8_t qsTxBuf[1024]; // buffer for QS transmit channel (QS-TX)
    static uint8_t qsRxBuf[128];  // buffer for QS receive channel (QS-RX)
    initBuf  (qsTxBuf, sizeof(qsTxBuf));
    rxInitBuf(qsRxBuf, sizeof(qsRxBuf));
    Serial.begin(115200); // run serial port at 115200 baud rate
    return true; // return success
}
//............................................................................
void QP::QS::onCommand(uint8_t cmdId, uint32_t param1,
                       uint32_t param2, uint32_t param3)
{
    (void)cmdId;
    (void)param1;
    (void)param2;
    (void)param3;
}

#endif // QS_ON

//............................................................................
void QP::QS::onCleanup(void) {
}
//............................................................................
QP::QSTimeCtr QP::QS::onGetTime(void) {
    return millis();
}
//............................................................................
void QP::QS::onFlush(void) {
#ifdef QS_ON
    uint16_t len = 0xFFFFU; // big number to get as many bytes as available
    uint8_t const *buf = QS::getBlock(&len); // get continguous block of data
    while (buf != nullptr) { // data available?
        Serial.write(buf, len); // might poll until all bytes fit
        len = 0xFFFFU; // big number to get as many bytes as available
        buf = QS::getBlock(&len); // try to get more data
    }
    Serial.flush(); // wait for the transmission of outgoing data to complete
#endif // QS_ON
}
//............................................................................
void QP::QS::onReset(void) {

}
