/*
 * SleepEvent.hpp
 *
 *  Created on: 28 апр. 2026 г.
 *      Author: Kirill
 */

#ifndef MAIN_EVENTMENAGER_EVENTS_SLEEPEVENT_HPP_
#define MAIN_EVENTMENAGER_EVENTS_SLEEPEVENT_HPP_

#include "IEvent.hpp"
#include "soc/gpio_num.h"
#include "EventTypes.hpp"
#include "freertos/FreeRTOS.h"

class SleepEvent  : public IEvent
{
  public:
    SleepEvent (gpio_num_t, QueueHandle_t*);
    virtual ~SleepEvent ();

  private:
    	static gpio_num_t isr_pin;
	    static event_cmd_t* isr_event;
	    static QueueHandle_t* global_event_queue;
	    static void IRAM_ATTR callback(void* arg);
};

#endif /* MAIN_EVENTMENAGER_EVENTS_SLEEPEVENT_HPP_ */
