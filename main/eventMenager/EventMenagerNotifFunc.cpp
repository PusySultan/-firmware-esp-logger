/*
 * EventMenagerNotifFunc.cpp
 *
 *  Created on: 2 мая 2026 г.
 *      Author: Kirill
 */

#include "EventMenager.hpp"
#include "NotificationTypes.hpp"
#include "pins_config_const.hpp"

void EventMenager :: notif_turn_on_device()
{
    notif_cmd_t* cmd_turn_on = new notif_cmd_t;

    cmd_turn_on -> event_type = NOTIFICATE;
    cmd_turn_on -> notif_source = LED_NOISE;
    cmd_turn_on -> led_gpio = GREEN_COLOR_PIN;
    cmd_turn_on -> blink_iteration = 10;

 	xQueueSend(*notif_event_queue, &cmd_turn_on, 0);   
}