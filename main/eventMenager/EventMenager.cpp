/*
 * EventMenager.cpp
 *
 *  Created on: 13 апр. 2026 г.
 *      Author: Kirill
 */
#include "CaseOpeningEvent.hpp"
#include "EventTypes.hpp"
#include "driver/gpio.h"
#include "convertFunc.hpp"
#include "NetworkTypes.hpp"
#include "EventMenager.hpp"
#include "UploaderTypes.hpp"
#include "DateTimeSensor.hpp"
#include "memory_w25q_const.hpp"
#include "pins_config_const.hpp"
#include "MessageCreaterTypes.hpp"
#include "MsgFromServerProcessors.hpp"

QueueHandle_t* EventMenager :: event_queue;
QueueHandle_t* EventMenager :: storage_event_queue;
QueueHandle_t* EventMenager :: sensor_event_queue;
QueueHandle_t* EventMenager :: upload_event_queue;
QueueHandle_t* EventMenager :: network_event_queue;
QueueHandle_t* EventMenager :: create_event_queue;
QueueHandle_t* EventMenager :: serverMsgProcessor_event_queue;

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
		return;
	}
	
	eventProcessors[cmd -> event_type](cmd);
	
}

void EventMenager :: fillFunctionMap()
{
	this -> eventProcessors[DEVICE_EVENT_ON] = [this](event_cmd_t* cmd)
	{
		this -> enebleTime = cmd -> dateTime;

		this -> connectGND();
				
		this -> saveEnableTime();
		
		this -> _caseOpeningEvent = new CaseOpeningEvent(EVENT_CASE_OPEN_PIN, event_queue);

		this -> createSensor(TEMP_SENSOR_1_ID);
		this -> createSensor(TEMP_SENSOR_2_ID);
		// this -> createSensor(TEMP_SENSOR_3_ID);
		// this -> createSensor(TEMP_SENSOR_C_ID);
		// this -> createSensor(DUST_SENSOR_2_ID);
		
		this -> initNetwork();
		this -> initUpload();	

		delete cmd;
	};
	
	this -> eventProcessors[DEVICE_EVENT_OFF] = [this](event_cmd_t* cmd)
	{
		printf("Start kill processes (EventMenager.eventProcessors[DEVICE_EVENT_OFF])\n");
				
		DateTimeSensor::getInstance().ds1302_getDateTime(&this -> disableTime);
		cmd -> dateTime = disableTime;
		
		// todo kill all process and go to sleep
		this -> KILL_PROCESS_FLSG = true;

		this -> killSensors();
		this -> killUpload();
		this -> killCreater();
		this -> killMsgProcessor();
		this -> killNetwork();

		this -> saveDisabeTime();
		this -> saveJobIntervalTime();
		this -> saveTotalJobTime();

		this -> killStorage();
		delete cmd;
	};	

	this -> eventProcessors[CASE_OPENING_EVENT] = [](event_cmd_t* cmd)
	{
		printf("\nCASE IS OPEN\n");

		DateTime dt;
		DateTimeSensor::getInstance().ds1302_getDateTime(&dt);

		create_cmd_t* create_cmd = new  create_cmd_t;

		create_cmd -> id = CASE_OPEN_SENSOR_ID;
		create_cmd -> cmd_type = CREATE;
		memcpy(create_cmd -> createrBlock[0].data, &dt, sizeof(dt));

		xQueueSend(*create_event_queue, create_cmd, 0);

		delete cmd;
	};

	this -> eventProcessors[CASE_CLOSENG_EVENT] = [](event_cmd_t* cmd)
	{
		printf("\nCASE IS CLOSE\n");

		DateTime dt;
		DateTimeSensor::getInstance().ds1302_getDateTime(&dt);

		create_cmd_t* create_cmd = new  create_cmd_t;

		create_cmd -> id = CASE_CLOSE_SENSOR_ID;
		create_cmd -> cmd_type = CREATE;
		memcpy(create_cmd -> createrBlock[0].data, &dt, sizeof(dt));

		xQueueSend(*create_event_queue, create_cmd, 0);

		delete cmd;
	};
}

void EventMenager :: overrideInternalQueue(QueueHandle_t* queue)
{
	event_queue = queue;
}

void EventMenager :: overrideStorageQueue(QueueHandle_t *queue)
{
	storage_event_queue = queue;
}

void EventMenager :: overrideSensorQueue(QueueHandle_t *queue)
{
	sensor_event_queue = queue;
}

void EventMenager :: overrideUploadQueue(QueueHandle_t *queue)
{
	upload_event_queue = queue;
}

void EventMenager :: overrideNetworkQueue(QueueHandle_t *queue)
{
	network_event_queue = queue;
}

void EventMenager :: overrideCreateQueue(QueueHandle_t *queue)
{
	create_event_queue = queue;
}

void EventMenager :: overrideSerMsgProcessQueue(QueueHandle_t* queue)
{
	serverMsgProcessor_event_queue = queue;
}

void EventMenager :: connectGND()
{
	 esp_rom_gpio_pad_select_gpio(static_cast<gpio_num_t>(ON_GND));
	 gpio_set_direction(static_cast<gpio_num_t>(ON_GND), GPIO_MODE_OUTPUT);
	 gpio_set_level(static_cast<gpio_num_t>(ON_GND), 1);	
}

void EventMenager :: saveEnableTime()
{
	char buffer[32];
	this -> enebleTime.toString(buffer);
	printf("Start date - time (EventMenager.eventProcessors[DEVICE_EVENT_ON]) %s\n", buffer);	
	
	// Записываем время включения
	block_data time_block;
	
	time_block.addr = ADDR_EVENT_DEVICE_ON;
	time_block.length = sizeof(enebleTime);
	memcpy(time_block.data, &enebleTime, sizeof(enebleTime));
	
	storage_cmd_t* stor_cmd = new storage_cmd_t;
	
	stor_cmd -> event_type = WRITE_DATA;
	stor_cmd -> sectorAddr = time_block.addr;
	stor_cmd -> sync_semaphore = NULL;
	stor_cmd -> data_size = 1;
	stor_cmd -> data[0] = time_block;
	
	xQueueSend(*storage_event_queue, &stor_cmd, 0);	
}

void EventMenager :: saveDisabeTime()
{
	block_data time_block;
	time_block.addr = ADDR_EVENT_DEVICE_OFF;
	time_block.length = sizeof(disableTime);
	memcpy(time_block.data, &disableTime, sizeof(disableTime));
	
	storage_cmd_t* reg_event_off = new storage_cmd_t;
	
	reg_event_off -> event_type = WRITE_BY_TRANS;
	reg_event_off -> sectorAddr = ADDR_EVENT_DEVICE_OFF;
	reg_event_off -> data_size = 1;
	reg_event_off -> data[0] = time_block;

	xQueueSendToFront(*storage_event_queue, reg_event_off, 0);
}

void EventMenager :: saveJobIntervalTime()
{
	DateTime interval = disableTime - enebleTime;
	
	block_data time_block;
	time_block.addr = ADDR_LAST_JOB_INTERVAL;
	time_block.length = sizeof(interval);
	memcpy(time_block.data, &interval, sizeof(interval));
	
	storage_cmd_t* reg_event_off = new storage_cmd_t;
	
	reg_event_off -> event_type = WRITE_BY_TRANS;
	reg_event_off -> sectorAddr =  ADDR_LAST_JOB_INTERVAL;
	reg_event_off -> data_size = 1;
	reg_event_off -> data[0] = time_block;

	xQueueSendToFront(*storage_event_queue, reg_event_off, 0);
}

void EventMenager :: saveTotalJobTime()
{
	DateTime lastTotalTime = getLastTotalJobTime();
	lastTotalTime += (disableTime - enebleTime);

	block_data time_block;
	time_block.addr = ADDR_TOTAL_JOB_TIME;
	time_block.length = sizeof(lastTotalTime);
	memcpy(time_block.data, &lastTotalTime, sizeof(lastTotalTime));
	
	storage_cmd_t* reg_event_off = new storage_cmd_t;
	
	reg_event_off -> event_type = WRITE_BY_TRANS;
	reg_event_off -> sectorAddr =  ADDR_TOTAL_JOB_TIME;
	reg_event_off -> data_size = 1;
	reg_event_off -> data[0] = time_block;

	xQueueSendToFront(*storage_event_queue, reg_event_off, 0);
}

DateTime EventMenager :: getLastTotalJobTime()
{
	DateTime lastTotalTime;
	storage_cmd_t* storage_cmd = new storage_cmd_t;
		
	storage_cmd -> event_type = READ_DATA;
	storage_cmd -> sync_semaphore = xSemaphoreCreateBinary();
	storage_cmd -> data_size = 1;
	storage_cmd -> sectorAddr = ADDR_TOTAL_JOB_TIME;
	
	storage_cmd -> data[0].length = sizeof(DateTime);
	storage_cmd -> data[0].addr = ADDR_TOTAL_JOB_TIME;

	xQueueSendToFront(*storage_event_queue, &storage_cmd, pdMS_TO_TICKS(30000));
	xSemaphoreTake(storage_cmd -> sync_semaphore, pdMS_TO_TICKS(30000));
	vSemaphoreDelete(storage_cmd -> sync_semaphore);

	if(arrayContainsTrush(storage_cmd -> data[0].data, storage_cmd -> data[0].length)) {
		lastTotalTime.fromBytes(0);
	} else {
		lastTotalTime.fromBytes(storage_cmd -> data[0].data);
	}

	delete storage_cmd;
	return lastTotalTime;
}

void EventMenager :: createSensor(SensorsID id)
{
	sensor_cmd_t* cmd_create_sensor = new sensor_cmd_t;
	
	cmd_create_sensor -> sensor_id  = id;
	cmd_create_sensor -> event_type = SENSOR_CREATE;
	cmd_create_sensor -> sync_semaphore = NULL;
	
	xQueueSend(*sensor_event_queue, &cmd_create_sensor, 0);	
}

void EventMenager :: killSensors()
{
	// Сначала удалим задачу датчиков чтобы не спамили другие задачи
	sensor_cmd_t* kill_process_sensor_cmd = new sensor_cmd_t;
	
	kill_process_sensor_cmd -> event_type = SHUTDOWN_SENSORS;
	kill_process_sensor_cmd -> sync_semaphore = xSemaphoreCreateBinary();
	
	xQueueSend(*sensor_event_queue, &kill_process_sensor_cmd, 10);
	xSemaphoreTake(kill_process_sensor_cmd -> sync_semaphore, pdMS_TO_TICKS(30000));
	vSemaphoreDelete(kill_process_sensor_cmd -> sync_semaphore);
	
	delete kill_process_sensor_cmd;
	
	printf("Sensors processes clean\n");
}

void EventMenager :: initNetwork()
{
	network_cmd_t* network_cmd = new network_cmd_t;
	
	network_cmd -> event_type = INIT_SETTINGS;
	network_cmd -> sync_semaphore = NULL;
	
	xQueueSend(*network_event_queue, &network_cmd, 0);
}

void EventMenager :: killNetwork()
{
	// kill Network
	network_cmd_t* kill_process_network_cmd = new network_cmd_t;
	kill_process_network_cmd -> event_type = SHUTDOWN_NETWORK;
	kill_process_network_cmd -> sync_semaphore = xSemaphoreCreateBinary();
	
	xQueueSend(*network_event_queue, &kill_process_network_cmd, 10);
	xSemaphoreTake(kill_process_network_cmd -> sync_semaphore, pdMS_TO_TICKS(30000));
	vSemaphoreDelete(kill_process_network_cmd -> sync_semaphore);
	
	delete kill_process_network_cmd;
}

void EventMenager :: initUpload()
{
	uploader_cmd_t* upload_cmd = new uploader_cmd_t;
	
	upload_cmd -> event_type = FIND_UPLOAD_ADDR;
	upload_cmd -> sync_semaphore = NULL;
	
	xQueueSend(*upload_event_queue, &upload_cmd, 0);
}

void EventMenager :: killUpload()
{
	// kill upload
	uploader_cmd_t* kill_process_upload_cmd = new uploader_cmd_t;
	
	kill_process_upload_cmd -> event_type = SHUTDOWN_UPLOAD;
	kill_process_upload_cmd -> sync_semaphore = xSemaphoreCreateBinary();
	
	xQueueSend(*upload_event_queue, &kill_process_upload_cmd, 10);
	xSemaphoreTake(kill_process_upload_cmd -> sync_semaphore, pdMS_TO_TICKS(30000));
	vSemaphoreDelete(kill_process_upload_cmd -> sync_semaphore);
	
	delete 	kill_process_upload_cmd;
	
	printf("Uploader processes clean\n");
}

void EventMenager :: killCreater()
{
	// kill creater
	create_cmd_t* kill_process_creater_cmd = new create_cmd_t;
	
	kill_process_creater_cmd -> cmd_type = SHUTDOWN_MSG_CREATER;
	kill_process_creater_cmd -> sync_semaphore = xSemaphoreCreateBinary();
	
	xQueueSend(*create_event_queue, &kill_process_creater_cmd, 10);
	xSemaphoreTake(kill_process_creater_cmd -> sync_semaphore, pdMS_TO_TICKS(30000));
	vSemaphoreDelete(kill_process_creater_cmd -> sync_semaphore);
	
	delete kill_process_creater_cmd;
	
	printf("Creater processes clean\n");
}

void EventMenager :: killMsgProcessor()
{
	// kill msgProcessor
	msg_server_cmd* kill_process_server_cmd = new msg_server_cmd;
	kill_process_server_cmd -> event_type = SHUTDOWN_SERVER_PROCESSOR;
	kill_process_server_cmd -> sync_semaphore = xSemaphoreCreateBinary();
	
	xQueueSend(*serverMsgProcessor_event_queue, &kill_process_server_cmd, 10);
	xSemaphoreTake(kill_process_server_cmd -> sync_semaphore, pdMS_TO_TICKS(30000));
	vSemaphoreDelete(kill_process_server_cmd -> sync_semaphore);
	
	delete kill_process_server_cmd;
}

void EventMenager :: killStorage()
{
	storage_cmd_t* kill_storage_process_cmd = new storage_cmd_t;
	kill_storage_process_cmd -> event_type = SHUTDOWN_STORAGE;
	kill_storage_process_cmd -> sync_semaphore = xSemaphoreCreateBinary();
	
	xQueueSend(*storage_event_queue, &kill_storage_process_cmd, 10);
	xSemaphoreTake(kill_storage_process_cmd -> sync_semaphore, pdMS_TO_TICKS(30000));
	vSemaphoreDelete(kill_storage_process_cmd -> sync_semaphore);
	
	delete kill_storage_process_cmd;
}