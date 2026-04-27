/*
 * eventFunc.hpp
 *
 *  Created on: 27 апр. 2026 г.
 *      Author: Kirill
 */

#ifndef MAIN_COMMONFUNCTIONS_EVENTFUNC_HPP_
#define MAIN_COMMONFUNCTIONS_EVENTFUNC_HPP_

#include "EventTypes.hpp"

inline char* event_type_to_string(event_type_t event_type)
{
	switch (event_type)
    {
        case DEVICE_EVENT_ON:       return "DEVICE_EVENT_ON";
        case DEVICE_EVENT_OFF:      return "DEVICE_EVENT_OFF";
        case CASE_OPENING_EVENT:    return "CASE_OPENING_EVENT";
        case CASE_CLOSENG_EVENT:    return "CASE_CLOSENG_EVENT";
        case DEVICE_EVENT_ERROR:    return "DEVICE_EVENT_ERROR";
        case DEVICE_EVENT_CRITICAL: return "DEVICE_EVENT_CRITICAL";

        default: return "DEVICE_EVENT_UNKNOW";
    }
} 

#endif /* MAIN_COMMONFUNCTIONS_EVENTFUNC_HPP_ */
