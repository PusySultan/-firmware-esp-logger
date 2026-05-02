/*
 * LedNotification.hpp
 *
 *  Created on: 2 мая 2026 г.
 *      Author: Kirill
 */

#ifndef MAIN_NOTIFICATION_NOTIFICATION_HPP_
#define MAIN_NOTIFICATION_NOTIFICATION_HPP_

#include <map>
#include <functional>
#include "freertos/FreeRTOS.h"
#include "NotificationTypes.hpp"
#include "pins_config_const.hpp"

class Notification final
{
    using func = std::function<void(notif_cmd_t*)>;

    public:
      Notification ();
      virtual ~Notification ();
      void notifProcessor(notif_cmd_t*);
      void static overrideInternalQueue(QueueHandle_t*);

    private:
      static QueueHandle_t* notification_event_queue;

      gpio_num_t led_pins[3] {RED_COLOR_PIN, BLUE_COLOR_PIN, GREEN_COLOR_PIN};
      std::map<notification_source_t, func> functionMap;

      void init();
      void fillNotifMap();
      void turn_off_pins_except(gpio_num_t);
};

#endif /* MAIN_NOTIFICATION_NOTIFICATION_HPP_ */
