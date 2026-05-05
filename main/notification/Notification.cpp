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

void Notification :: notifProcessor(notif_cmd_t* cmd)
{
	if(!cmd) {
		printf("error cmd in NULL (Notification.eventProcessor)\n");
		delete cmd;
		return;
	}

	if(cmd -> event_type < 0) {
		printf("error incorrect cmd type (Notification.eventProcessor)\n");

		if(cmd -> sync_semaphore && cmd -> sync_semaphore != NULL) {
			xSemaphoreGive(cmd -> sync_semaphore);
			return;
		}

		delete cmd;
		return;
	}

	if(cmd -> event_type == SHUTDOWN_NOTIF){

		this -> deInitLed();
		// this -> deinitBuzzrer();

		if(cmd -> sync_semaphore && cmd -> sync_semaphore != NULL) {
			xSemaphoreGive(cmd -> sync_semaphore);
			return;
		}

		delete cmd;
		return;
	}

	if(!functionMap.contains(cmd -> notif_source)) {
		printf("error has not func with id: %d (Notification.eventProcessor)\n", cmd -> event_type);

		if(cmd -> sync_semaphore && cmd -> sync_semaphore != NULL) {
			xSemaphoreGive(cmd -> sync_semaphore);
			return;
		}

		delete cmd;
		return;
	}

	functionMap[cmd -> notif_source](cmd);
}

void Notification :: fillNotifMap()
{
	functionMap[LED] = [this] (notif_cmd_t* cmd)
	{
		this -> turn_off_pins_except(cmd -> led_gpio);

		bool level = 1;

		for(uint8_t i = 0; i <= cmd -> blink_iteration; i++)
		{
			gpio_set_level(cmd -> led_gpio, level ? 1 : 0);
			vTaskDelay(10);

			level = !level;
		}

		gpio_set_level(cmd -> led_gpio, 0);

		if(cmd -> sync_semaphore && cmd -> sync_semaphore != NULL) {
			xSemaphoreGive(cmd -> sync_semaphore);
			return;
		}

		delete cmd;
	};

	functionMap[NOISE] = [] (notif_cmd_t* cmd)
	{
		ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, DUTY);
		ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);

		for(uint8_t i = 0; i <= cmd -> blink_iteration; i++)
		{
			vTaskDelay(10);
		}

		ledc_timer_pause(LEDC_MODE, LEDC_TIMER);

		if(cmd -> sync_semaphore && cmd -> sync_semaphore != NULL) {
			xSemaphoreGive(cmd -> sync_semaphore);
			return;
		}

		delete cmd;
	};

	functionMap[LED_NOISE] = [this] (notif_cmd_t* cmd)
	{
		this -> turn_off_pins_except(cmd -> led_gpio);

		bool level = 1;
		ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, DUTY);
		ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);

		for(uint8_t i = 0; i <= cmd -> blink_iteration; i++)
		{
			// gpio_set_level(GPIO_NUM_12, 1);

			if(level) {
				gpio_set_level(cmd -> led_gpio, 1);
				// ledc_timer_resume(LEDC_MODE, LEDC_TIMER);
			} else{
				// ledc_timer_pause(LEDC_MODE, LEDC_TIMER);
				gpio_set_level(cmd -> led_gpio, 0);
			}
			
			vTaskDelay(10);
			level = !level;
		}

		ledc_timer_pause(LEDC_MODE, LEDC_TIMER);

		// gpio_set_level(GPIO_NUM_12, 1);
		gpio_set_level(cmd -> led_gpio, 0);

		if(cmd -> sync_semaphore && cmd -> sync_semaphore != NULL) {
			xSemaphoreGive(cmd -> sync_semaphore);
			return;
		}

		delete cmd;
	};
}

void Notification :: turn_off_pins_except(gpio_num_t gpio_num_except)
{
	for(uint8_t i = 0; i < 3; i++)
	{
		if(led_pins[i] != gpio_num_except) continue;
		gpio_set_level(led_pins[i], 0);
	}
}

void Notification :: init()
{
	this -> initLed();
	this -> initBuzzer();
}

void Notification :: initLed()
{
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

void Notification :: initBuzzer()
{
	  // Таймер
    ledc_timer_config_t timer_cfg = {
        .speed_mode = LEDC_MODE,           // LEDC_HIGH_SPEED_MODE
        .duty_resolution = LEDC_TIMER_10_BIT,
        .timer_num = LEDC_TIMER,           // LEDC_TIMER_0
        .freq_hz = 4000,
        .clk_cfg = LEDC_AUTO_CLK
    };

    esp_err_t err = ledc_timer_config(&timer_cfg);
    if (err != ESP_OK) {
        printf("ledc_timer_config failed: %s", esp_err_to_name(err));
        return;
    }

    // Канал
    ledc_channel_config_t ch_cfg = {
        .gpio_num = BUZZER_PIN,
        .speed_mode = LEDC_MODE,
        .channel = LEDC_CHANNEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER,
        .duty = DUTY,
        .hpoint = 0
    };

    err = ledc_channel_config(&ch_cfg);
    if (err != ESP_OK) {
        printf("ledc_channel_config failed: %s", esp_err_to_name(err));
        return;
    }

    // По желанию: сразу приостановить таймер, чтобы не пищал
    ledc_timer_pause(LEDC_MODE, LEDC_TIMER);
}

void Notification :: deInitLed()
{
	gpio_num_t gpio_arr[] {RED_COLOR_PIN, GREEN_COLOR_PIN};

	for(int i = 0; i < 3; i++)
	{
		gpio_config_t io_conf = {
			.pin_bit_mask = (1ULL << gpio_arr[i]),
			.mode = GPIO_MODE_INPUT,          // Режим входа
			.pull_up_en = GPIO_PULLUP_DISABLE, // Отключить подтяжку вверх
			.pull_down_en = GPIO_PULLDOWN_DISABLE, // Отключить подтяжку вниз
			.intr_type = GPIO_INTR_DISABLE    // Отключить прерывания
    	};
    	gpio_config(&io_conf);
	}
}

void Notification :: deinitBuzzrer()
{
	  // 1. Останавливаем ШИМ и устанавливаем пин в 0
    ledc_stop(LEDC_MODE, LEDC_CHANNEL, 0); 
    
    // 2. Приостанавливаем таймер (обязательное условие)
    ledc_timer_pause(LEDC_MODE, LEDC_TIMER); 
    
    // 3. Деинициализируем таймер через флаг deconfigure
    ledc_timer_config_t timer_del_cfg = {
        .speed_mode = LEDC_MODE,
        .timer_num = LEDC_TIMER,
        .deconfigure = true,
    };
    ledc_timer_config(&timer_del_cfg);
    
    // 4. Сбрасываем пин в состояние по умолчанию
    gpio_reset_pin(static_cast<gpio_num_t>(BUZZER_PIN));
}