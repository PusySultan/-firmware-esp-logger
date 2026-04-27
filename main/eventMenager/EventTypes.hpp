/*
 * EventTypes.hpp
 *
 *  Created on: 13 апр. 2026 г.
 *      Author: Kirill
 */

#ifndef MAIN_EVENTMENAGER_EVENTTYPES_HPP_
#define MAIN_EVENTMENAGER_EVENTTYPES_HPP_

#include "DateTime.hpp"
enum event_type_t
{
	DEVICE_EVENT_UNKNOW = 0,

	DEVICE_EVENT_ON  = 1,
	DEVICE_EVENT_OFF = 2,
	
	DEVICE_EVENT_ERROR = 3,
	DEVICE_EVENT_CRITICAL = 4,
	
	CASE_OPENING_EVENT = 5,
	CASE_CLOSENG_EVENT = 6,
};

struct event_cmd_t
{
	event_type_t event_type;
	DateTime dateTime;
};


#endif /* MAIN_EVENTMENAGER_EVENTTYPES_HPP_ */
