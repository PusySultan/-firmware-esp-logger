/*
 * LedNotification.cpp
 *
 *  Created on: 2 мая 2026 г.
 *      Author: Kirill
 */

#include "driver/gpio.h"
#include "soc/gpio_num.h"
#include "Notification.hpp"
#include "pins_config_const.hpp"

QueueHandle_t* Notification :: notification_event_queue;

Notification :: Notification ()
{
	this -> init();
	this -> fillNotifMap();
}

Notification :: ~Notification ()
{
  // TODO Auto-generated destructor stub
}

void Notification :: overrideInternalQueue(QueueHandle_t* queue)
{
	notification_event_queue = queue;
}

void Notification :: fillNotifMap()
{
	functionMap[LED] = [this] (notif_cmd_t* cmd)
	{
		this -> turn_off_pins_except(cmd -> led_gpio);
		gpio_set_level(cmd -> led_gpio, 1);

		vTaskDelay(cmd -> duration_ms);
	};

	functionMap[NOISE] = [] (notif_cmd_t* cmd)
	{
		
	};

	functionMap[LED_NOISE] = [this] (notif_cmd_t* cmd)
	{
		this -> turn_off_pins_except(cmd -> led_gpio);
		gpio_set_level(cmd -> led_gpio, 1);

		vTaskDelay(cmd -> duration_ms);
	};

}

void Notification :: turn_off_pins_except(gpio_num_t gpio_num_except)
{
	for(uint8_t i = 0; i < 3; i++)
	{
		if(gpio_num_except != gpio_num_except) continue;
		gpio_set_level(led_pins[i], 0);
	}
}

void Notification :: init()
{
	// todo init Buzzer

	gpio_reset_pin(RED_COLOR_PIN);
	gpio_reset_pin(GREEN_COLOR_PIN);
	gpio_reset_pin(BLUE_COLOR_PIN);

	gpio_set_direction(RED_COLOR_PIN, GPIO_MODE_OUTPUT);
	gpio_set_direction(GREEN_COLOR_PIN, GPIO_MODE_OUTPUT);
	gpio_set_direction(BLUE_COLOR_PIN, GPIO_MODE_OUTPUT);
		
	gpio_set_level(RED_COLOR_PIN, 0);
	gpio_set_level(GREEN_COLOR_PIN, 0);
	gpio_set_level(BLUE_COLOR_PIN, 0);
}