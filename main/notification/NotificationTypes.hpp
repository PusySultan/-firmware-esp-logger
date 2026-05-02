/*
 * NotificationTypes.hpp
 *
 *  Created on: 2 мая 2026 г.
 *      Author: Kirill
 */

#ifndef MAIN_NOTIFICATION_NOTIFICATIONTYPES_HPP_
#define MAIN_NOTIFICATION_NOTIFICATIONTYPES_HPP_

#include <stdint.h>
#include "soc/gpio_num.h"

enum notification_event_type_t
{
    SHUTDOWN_NOTIF = 0,
    INIT_NOTIF = 1,
    NOTIFICATE = 2
};

enum notification_source_t
{
    LED = 1,
    NOISE = 2,
    LED_NOISE = 3
};

struct notif_cmd_t
{
    notification_event_type_t event_type;
    notification_source_t notif_source;
    uint8_t blink_iteration;

	gpio_num_t led_gpio;    
};


#endif /* MAIN_NOTIFICATION_NOTIFICATIONTYPES_HPP_ */
