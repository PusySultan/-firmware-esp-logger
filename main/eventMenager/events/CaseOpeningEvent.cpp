/*
 * CaseOpeningEvent.cpp
 *
 *  Created on: 26 апр. 2026 г.
 *      Author: Kirill
 */

#include "CaseOpeningEvent.hpp"
#include "driver/gpio.h"
#include "pins_config_const.hpp"

QueueHandle_t* CaseOpeningEvent :: global_event_queue;
event_cmd_t* CaseOpeningEvent :: isr_event;
gpio_num_t CaseOpeningEvent :: isr_pin;
bool CaseOpeningEvent :: CASE_IS_OPEN;

CaseOpeningEvent :: CaseOpeningEvent (gpio_num_t PIN, QueueHandle_t* _global_event_queue) : IEvent()
{
	CASE_IS_OPEN = false;

	global_event_queue = _global_event_queue;
	isr_pin = PIN;

	gpio_config_t gp_config = {
		.pin_bit_mask = (1ULL << isr_pin),
		.mode = GPIO_MODE_INPUT,
		.pull_up_en = GPIO_PULLUP_DISABLE,
		.pull_down_en = GPIO_PULLDOWN_DISABLE,
		.intr_type = GPIO_INTR_ANYEDGE		    // Прерывание при любом уровне (по фронту)
	};
		
	gpio_config(&gp_config);
		
	gpio_install_isr_service(0);
	gpio_isr_handler_add(isr_pin, callback, NULL);
}

CaseOpeningEvent::~CaseOpeningEvent ()
{
  	gpio_isr_handler_remove(isr_pin);
}

void IRAM_ATTR CaseOpeningEvent :: callback(void* arg)
{
	  // Очищаем флаг прерывания GPIO
    // gpio_ll_clear_intr_status(GPIO_PIN_MASK[VOLTAGE_OBSERVER_PIN]);
    
	gpio_intr_disable(isr_pin);
	
	isr_event = new event_cmd_t;
	isr_event -> event_type = CASE_IS_OPEN ? CASE_CLOSENG_EVENT : CASE_OPENING_EVENT;
	
	CASE_IS_OPEN = !CASE_IS_OPEN;

	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	xQueueSendFromISR(*global_event_queue, &isr_event, &xHigherPriorityTaskWoken);
}

void CaseOpeningEvent :: interruptEnable()
{
	gpio_intr_enable(isr_pin);
}



