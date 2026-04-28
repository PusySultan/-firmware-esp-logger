/*
 * CaseOpeningEvent.hpp
 *
 *  Created on: 26 апр. 2026 г.
 *      Author: Kirill
 */

#ifndef MAIN_EVENTMENAGER_EVENTS_CASEOPENINGEVENT_HPP_
#define MAIN_EVENTMENAGER_EVENTS_CASEOPENINGEVENT_HPP_

#include "IEvent.hpp"
#include "soc/gpio_num.h"
#include "EventTypes.hpp"
#include "freertos/FreeRTOS.h"

class CaseOpeningEvent : public IEvent
{
	public:
	  CaseOpeningEvent (gpio_num_t, QueueHandle_t*);
	  virtual ~CaseOpeningEvent ();

	  void interruptEnable();

	private:
		static bool CASE_IS_OPEN;

		static gpio_num_t isr_pin;
		static event_cmd_t* isr_event;
		static QueueHandle_t* global_event_queue;
		static void IRAM_ATTR callback(void* arg);
};

#endif /* MAIN_EVENTMENAGER_EVENTS_CASEOPENINGEVENT_HPP_ */
