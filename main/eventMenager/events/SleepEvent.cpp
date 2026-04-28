/*
 * SleepEvent.cpp
 *
 *  Created on: 28 апр. 2026 г.
 *      Author: Kirill
 */

#include "SleepEvent.hpp"
#include "driver/gpio.h"

QueueHandle_t* SleepEvent :: global_event_queue;
event_cmd_t* SleepEvent :: isr_event;
gpio_num_t SleepEvent :: isr_pin;

SleepEvent :: SleepEvent (gpio_num_t PIN, QueueHandle_t* _global_event_queue) : IEvent()
{
    global_event_queue = _global_event_queue;
	 isr_pin = PIN;

    gpio_config_t gp_config = {
        .pin_bit_mask = (1ULL << isr_pin),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE		    // Прерывание 1 -> 0
    };

    gpio_config(&gp_config);
    gpio_isr_handler_add(isr_pin, callback, NULL);
}

SleepEvent :: ~SleepEvent ()
{
  // TODO Auto-generated destructor stub
}

void IRAM_ATTR SleepEvent:: callback(void* arg)
{
	  // Очищаем флаг прерывания GPIO
    // gpio_ll_clear_intr_status(GPIO_PIN_MASK[VOLTAGE_OBSERVER_PIN]);
    
	gpio_intr_disable(isr_pin);
	
	isr_event = new event_cmd_t;
	isr_event -> event_type = DEVICE_EVENT_OFF;
	
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	xQueueSendFromISR(*global_event_queue, &isr_event, &xHigherPriorityTaskWoken);
}

