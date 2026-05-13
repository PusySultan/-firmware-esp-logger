/*
 * EventMenager.cpp
 *
 *  Created on: 13 апр. 2026 г.
 *      Author: Kirill
 */
#include "SleepEvent.hpp"
#include "convertFunc.hpp"
#include "EventMenager.hpp"
#include "DateTimeSensor.hpp"
#include "CaseOpeningEvent.hpp"
#include "hal/gpio_types.h"
#include "memory_w25q_const.hpp"
#include "pins_config_const.hpp"
#include "MessageCreaterTypes.hpp"
#include "MsgFromServerProcessors.hpp"
#include "EventMenagerTimeFunc.cpp"
#include "driver/gpio.h"

EventMenager :: EventMenager ()
{
	fillFunctionMap();
}

EventMenager :: ~EventMenager ()
{
  // TODO Auto-generated destructor stub
}

void EventMenager :: eventProcessor(event_cmd_t* cmd)
{
	if(!cmd)
	{
		printf("ERROR cmd is NULL, (EventMenager.eventProcessors)\n");
		return;
	}
	
	if(cmd -> event_type < 0 || !this -> eventProcessors.contains(cmd -> event_type))
	{
		printf("ERROR cmd event type is BAD, (EventMenager.eventProcessor)\n");
		delete cmd;
		return;
	}
	
	eventProcessors[cmd -> event_type](cmd);
	
}

void EventMenager :: fillFunctionMap()
{
	this -> eventProcessors[DEVICE_EVENT_ON] = [this](event_cmd_t* cmd)
	{
		this -> enebleTime = cmd -> dateTime;
		this -> notif_turn_on_device();

		this -> connectGND();
		this -> initMsgCreater();
		this -> saveEnableEvent();
		
		this -> _caseOpeningEvent = new CaseOpeningEvent(EVENT_CASE_OPEN_PIN, event_queue);
		this -> _voltageOffEvent = new SleepEvent(EVENT_VOLTAGE_OFF_PIN ,event_queue);

		this -> createSensor(TEMP_SENSOR_1_ID);
		this -> createSensor(TEMP_SENSOR_2_ID);
		this -> createSensor(TEMP_SENSOR_3_ID);
		// this -> createSensor(TEMP_SENSOR_C_ID);
		this -> createSensor(DUST_SENSOR_1_ID);
		// this -> createSensor(DUST_SENSOR_2_ID);
		
		this -> initNetwork();
		this -> initUpload();	

		delete cmd;
	};
	
	this -> eventProcessors[DEVICE_EVENT_OFF] = [this](event_cmd_t* cmd)
	{
		printf("\n\n\nStart kill processes (EventMenager.eventProcessors[DEVICE_EVENT_OFF])\n\n\n");
		
		DateTimeSensor::getInstance().ds1302_getDateTime(&this -> disableTime);
		cmd -> dateTime = disableTime;
		
		this -> KILL_PROCESS_FLSG = true;

		this -> notif_turn_off_pin();
		//this -> killNotif();

		// this -> killSensors();
		// this -> killUpload();
		// this -> killCreater();
		// this -> killMsgProcessor();
		// this -> killNetwork();

		this -> saveDisabeEvent();
		this -> saveJobIntervalTime();
		this -> saveTotalJobTime();

		// this -> killStorage();

		vTaskDelay(pdMS_TO_TICKS(5000));
		delete cmd;
	};	

	this -> eventProcessors[CASE_OPENING_EVENT] = [this](event_cmd_t* cmd)
	{
		printf("\nCASE IS OPEN\n");
		// Сохраняем время вскрыти также в коде, для того чтобы после не читать его из памяти
		DateTimeSensor::getInstance().ds1302_getDateTime(&caseOpenTime);

		create_cmd_t* create_cmd = new  create_cmd_t;

		create_cmd -> collection = DEVICE_EVENT;
		create_cmd -> sync_semaphore = NULL;
		create_cmd -> cmd_type = CREATE;
		create_cmd -> dateTime = caseOpenTime;

		create_cmd -> block_length = 1;
		create_cmd -> createrBlock[0].byte_count = sizeof(CASE_OPENING_EVENT);
		writeU32LE(create_cmd -> createrBlock[0].data, CASE_OPENING_EVENT);
		xQueueSend(*create_event_queue, &create_cmd, 0);

		storage_cmd_t* save_event = new storage_cmd_t;

		save_event -> event_type = WRITE_BY_TRANS;
		save_event -> sectorAddr = ADDR_EVENT_CASE_OPEN;
		save_event -> sync_semaphore = NULL;
		save_event -> data_size = 2;

		// data
		save_event -> data[0].addr = ADDR_EVENT_CASE_OPEN;
		save_event -> data[0].length = sizeof(CASE_OPENING_EVENT);	// 4
		writeU32LE(save_event -> data[0].data, CASE_OPENING_EVENT);
		
		// dateTime
		save_event -> data[1].addr = ADDR_EVENT_CASE_OPEN + save_event -> data[0].length;
		save_event -> data[1].length = sizeof(caseOpenTime);
		memcpy(save_event -> data[1].data, &caseOpenTime, save_event -> data[1].length);
		xQueueSend(*storage_event_queue, &save_event, 0);

		vTaskDelay(pdMS_TO_TICKS(3000));
		this -> _caseOpeningEvent -> interruptEnable();
		delete cmd;

		this -> notif_case_open();
	};

	this -> eventProcessors[CASE_CLOSENG_EVENT] = [this](event_cmd_t* cmd)
	{
		printf("\nCASE IS CLOSE\n");

		DateTime dt;
		DateTimeSensor::getInstance().ds1302_getDateTime(&dt);

		create_cmd_t* create_cmd = new  create_cmd_t;

		create_cmd -> collection = DEVICE_EVENT;
		create_cmd -> sync_semaphore = NULL;
		create_cmd -> cmd_type = CREATE;
		create_cmd -> dateTime = dt;

		create_cmd -> block_length = 1;
		create_cmd -> createrBlock[0].byte_count = sizeof(CASE_CLOSENG_EVENT);
		writeU32LE(create_cmd -> createrBlock[0].data, CASE_CLOSENG_EVENT);
		xQueueSend(*create_event_queue, &create_cmd, 0);

		storage_cmd_t* save_event = new storage_cmd_t;

		save_event -> event_type = WRITE_BY_TRANS;
		save_event -> sectorAddr = ADDR_EVENT_CASE_CLOSE;
		save_event -> sync_semaphore = NULL;
		save_event -> data_size = 1;

		// data
		save_event -> data[0].addr = ADDR_EVENT_CASE_CLOSE;
		save_event -> data[0].length = sizeof(CASE_CLOSENG_EVENT);	// 4
		writeU32LE(save_event -> data[0].data, CASE_CLOSENG_EVENT);
		
		// dateTime
		save_event -> data[1].addr = ADDR_EVENT_CASE_CLOSE + save_event -> data[0].length;
		save_event -> data[1].length = sizeof(dt);
		memcpy(save_event -> data[1].data, &dt, save_event -> data[1].length);
		xQueueSend(*storage_event_queue, &save_event, 0);

		vTaskDelay(pdMS_TO_TICKS(3000));
		this -> _caseOpeningEvent -> interruptEnable();
		
		delete cmd;
	};
}

void EventMenager :: connectGND()
{
	 esp_rom_gpio_pad_select_gpio(static_cast<gpio_num_t>(ON_GND));
	 gpio_set_direction(static_cast<gpio_num_t>(ON_GND), GPIO_MODE_OUTPUT);
	 gpio_set_level(static_cast<gpio_num_t>(ON_GND), 1);	
}