/*
 * EventMenagerInitFunc.cpp
 *
 *  Created on: 30 апр. 2026 г.
 *      Author: Kirill
 */


#include "EventMenager.hpp"
#include "NetworkTypes.hpp"
#include "UploaderTypes.hpp"
#include "MessageCreaterTypes.hpp"

void EventMenager :: initMsgCreater()
{
	create_cmd_t* cretae_init_cmd = new create_cmd_t;

	cretae_init_cmd -> cmd_type = INIT;
	cretae_init_cmd -> sync_semaphore = xSemaphoreCreateBinary();

	xQueueSend(*create_event_queue, &cretae_init_cmd, 0);
	xSemaphoreTake(cretae_init_cmd -> sync_semaphore, portMAX_DELAY);
	vSemaphoreDelete(cretae_init_cmd -> sync_semaphore);
}

void EventMenager :: initNetwork()
{
	network_cmd_t* network_cmd = new network_cmd_t;
	
	network_cmd -> event_type = INIT_SETTINGS;
	network_cmd -> sync_semaphore = NULL;
	
	xQueueSend(*network_event_queue, &network_cmd, 0);
}

void EventMenager :: createSensor(SensorsID id)
{
	sensor_cmd_t* cmd_create_sensor = new sensor_cmd_t;
	
	cmd_create_sensor -> sensor_id  = id;
	cmd_create_sensor -> event_type = SENSOR_CREATE;
	cmd_create_sensor -> sync_semaphore = NULL;
	
	xQueueSend(*sensor_event_queue, &cmd_create_sensor, 0);
}

void EventMenager :: initUpload()
{
	uploader_cmd_t* upload_cmd = new uploader_cmd_t;
	
	upload_cmd -> event_type = FIND_UPLOAD_ADDR;
	upload_cmd -> sync_semaphore = NULL;
	
	xQueueSend(*upload_event_queue, &upload_cmd, 0);
}
