#include "qpcpp.hpp"
#include "qf_port.hpp"
#include "esp_log.h"
#include "esp_freertos_hooks.h"

static const char * TAG = "qf_hooks";

static uint8_t const l_TickHook = static_cast<uint8_t>(0);
    
static void IRAM_ATTR  tickHook_ESP32(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    /* process time events for rate 0 */
    QP::QF::TICK_FROM_ISR(&xHigherPriorityTaskWoken, &l_TickHook);
    /* notify FreeRTOS to perform context switch from ISR, if needed */
    if(xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
}

void QP::QF::onStartup(void) {
	esp_register_freertos_tick_hook_for_cpu(tickHook_ESP32, QP_CPU_NUM);
    QS_OBJ_DICTIONARY(&l_TickHook);
}


extern "C" Q_NORETURN Q_onAssert(char_t const * const module, int_t location)
{
    ESP_LOGE(TAG, "Q_onAssert: module:%s loc:%d\n", module, location);
    while(1);
    
}
