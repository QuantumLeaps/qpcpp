
/*
* Copyright (C) 2022 Victor Chavez
* This program is open source software: you can redistribute it and/or
* modify it under the terms of the GNU General Public License as published
* by the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* Alternatively, this program may be distributed and modified under the
* terms of Quantum Leaps commercial licenses, which expressly supersede
* the GNU General Public License and are specifically designed for
* licensees interested in retaining the proprietary status of their code.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <www.gnu.org/licenses>.
*/
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
