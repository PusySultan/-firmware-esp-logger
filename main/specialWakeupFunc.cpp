/*
 * specialWakeupFunc.cpp
 *
 *  Created on: 6 мая 2026 г.
 *      Author: Kirill
 */


#include "EventTypes.hpp"
#include "convertFunc.hpp"
#include "main.hpp"
#include "DateTimeSensor.hpp"
#include "memory_w25q_const.hpp"

void regCaseOpeningInSleepMode()
{
	DateTime dateTime;
	DateTimeSensor :: getInstance().ds1302_getDateTime(&dateTime);

	storage_cmd_t* storage_cmd = new storage_cmd_t;
		
	storage_cmd -> event_type = WRITE_DATA;
	storage_cmd -> data_size  = 2;
	storage_cmd -> sectorAddr = ADDR_EVENT_CASE_OPEN;
    storage_cmd -> sync_semaphore = xSemaphoreCreateBinary();
	
	storage_cmd -> data[0].addr = ADDR_EVENT_CASE_OPEN;
	storage_cmd -> data[0].length = sizeof(CASE_OPENING_EVENT);
	writeU32LE(storage_cmd -> data[0].data, CASE_OPENING_EVENT);

	storage_cmd -> data[1].addr = ADDR_EVENT_CASE_OPEN + 4;
	storage_cmd -> data[1].length = sizeof(dateTime);
	memcpy(storage_cmd -> data[1].data, &dateTime, sizeof(dateTime));

	xQueueSend(storage_event_queue, &storage_cmd,  pdMS_TO_TICKS(10000));
}

void notifCaseOpenInSleepMode()
{
    notif_cmd_t* cmd_turn_on = new notif_cmd_t;

    cmd_turn_on -> event_type = NOTIFICATE;
    cmd_turn_on -> notif_source = LED_NOISE;
    cmd_turn_on -> led_gpio = RED_COLOR_PIN;
    cmd_turn_on -> blink_iteration = 20;
    cmd_turn_on -> sync_semaphore = xSemaphoreCreateBinary();

 	xQueueSend(notif_event_queue, &cmd_turn_on, pdMS_TO_TICKS(10000));
	xSemaphoreTake(cmd_turn_on -> sync_semaphore, pdMS_TO_TICKS(10000));
	vSemaphoreDelete(cmd_turn_on -> sync_semaphore);

	delete cmd_turn_on;
}
void startSpecialInit()
{
	connectGND();

	storage = new Storage();
	myNotif = new Notification();

	notif_event_queue = xQueueCreate(1, sizeof(notif_cmd_t*));
	storage_event_queue = xQueueCreate(1,  sizeof(storage_cmd_t*));

	myNotif -> overrideInternalQueue(&notif_event_queue);
	storage -> overrideInternalQueue(&storage_event_queue);

	createSpecialTasks();

    DateTime dt;
	DateTimeSensor :: getInstance().ds1302_init(CLOCK_ENA_PIN, CLOCK_CLK_PIN, CLOCK_DAT_PIN); // 26, 14, 27
	DateTimeSensor :: DateTimeSensor::getInstance().ds1302_getDateTime(&dt);

    notifCaseOpenInSleepMode();
    regCaseOpeningInSleepMode();

	byby();
}

void connectGND()
{
    esp_rom_gpio_pad_select_gpio(static_cast<gpio_num_t>(ON_GND));
	gpio_set_direction(static_cast<gpio_num_t>(ON_GND), GPIO_MODE_OUTPUT);
	gpio_set_level(static_cast<gpio_num_t>(ON_GND), 1);
}