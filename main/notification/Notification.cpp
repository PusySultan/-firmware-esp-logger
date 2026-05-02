/*
 * LedNotification.cpp
 *
 *  Created on: 2 мая 2026 г.
 *      Author: Kirill
 */

#include "Notification.hpp"

Notification :: Notification ()
{
  // TODO Auto-generated constructor stub

}

Notification::~Notification ()
{
  // TODO Auto-generated destructor stub
}

/*
		gpio_reset_pin(RED_COLOR_PIN);
		gpio_reset_pin(GREEN_COLOR_PIN);
		gpio_reset_pin(BLUE_COLOR_PIN);

		gpio_set_direction(RED_COLOR_PIN, GPIO_MODE_OUTPUT);
		gpio_set_direction(GREEN_COLOR_PIN, GPIO_MODE_OUTPUT);
		gpio_set_direction(BLUE_COLOR_PIN, GPIO_MODE_OUTPUT);
		
		gpio_set_level(RED_COLOR_PIN, 0);
		gpio_set_level(GREEN_COLOR_PIN, 0);
		gpio_set_level(BLUE_COLOR_PIN, 0);

		printf("On red\n");
		gpio_set_level(RED_COLOR_PIN, 1);
		
		vTaskDelay(pdMS_TO_TICKS(5000));
		gpio_set_level(RED_COLOR_PIN, 0);

		printf("On green\n");
		gpio_set_level(GREEN_COLOR_PIN, 1);

		vTaskDelay(pdMS_TO_TICKS(5000));
		gpio_set_level(GREEN_COLOR_PIN, 0);

		printf("On blue\n");
		gpio_set_level(BLUE_COLOR_PIN, 1);
		
		vTaskDelay(pdMS_TO_TICKS(5000));
		gpio_set_level(BLUE_COLOR_PIN, 0);
*/
