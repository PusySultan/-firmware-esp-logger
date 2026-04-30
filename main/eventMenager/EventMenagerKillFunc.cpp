/*
 * EventMenagerKillFunc.cpp
 *
 *  Created on: 30 апр. 2026 г.
 *      Author: Kirill
 */


#include "EventMenager.hpp"
#include "MsgServerTypes.hpp"
#include "NetworkTypes.hpp"
#include "StorageTypes.hpp"
#include "UploaderTypes.hpp"
#include "MessageCreaterTypes.hpp"

void EventMenager :: killSensors()
{
	// Сначала удалим задачу датчиков чтобы не спамили другие задачи
	sensor_cmd_t* kill_process_sensor_cmd = new sensor_cmd_t;
	
	kill_process_sensor_cmd -> event_type = SHUTDOWN_SENSORS;
	kill_process_sensor_cmd -> sync_semaphore = xSemaphoreCreateBinary();
	
	xQueueSend(*sensor_event_queue, &kill_process_sensor_cmd, pdMS_TO_TICKS(10000));
	xSemaphoreTake(kill_process_sensor_cmd -> sync_semaphore, pdMS_TO_TICKS(30000));
	vSemaphoreDelete(kill_process_sensor_cmd -> sync_semaphore);
	
	delete kill_process_sensor_cmd;
	
	printf("Sensors processes clean\n");
}

void EventMenager :: killNetwork()
{
	// kill Network
	network_cmd_t* kill_process_network_cmd = new network_cmd_t;
	kill_process_network_cmd -> event_type = SHUTDOWN_NETWORK;
	kill_process_network_cmd -> sync_semaphore = xSemaphoreCreateBinary();
	
	xQueueSend(*network_event_queue, &kill_process_network_cmd, pdMS_TO_TICKS(10000));
	xSemaphoreTake(kill_process_network_cmd -> sync_semaphore, pdMS_TO_TICKS(30000));
	vSemaphoreDelete(kill_process_network_cmd -> sync_semaphore);
	
	delete kill_process_network_cmd;
}

void EventMenager :: killUpload()
{
	// kill upload
	uploader_cmd_t* kill_process_upload_cmd = new uploader_cmd_t;
	
	kill_process_upload_cmd -> event_type = SHUTDOWN_UPLOAD;
	kill_process_upload_cmd -> sync_semaphore = xSemaphoreCreateBinary();
	
	xQueueSend(*upload_event_queue, &kill_process_upload_cmd, pdMS_TO_TICKS(10000));
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
	
	xQueueSend(*create_event_queue, &kill_process_creater_cmd, pdMS_TO_TICKS(10000));
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
	
	xQueueSend(*serverMsgProcessor_event_queue, &kill_process_server_cmd, pdMS_TO_TICKS(10000));
	xSemaphoreTake(kill_process_server_cmd -> sync_semaphore, pdMS_TO_TICKS(30000));
	vSemaphoreDelete(kill_process_server_cmd -> sync_semaphore);
	
	delete kill_process_server_cmd;
}

void EventMenager :: killStorage()
{
	storage_cmd_t* kill_storage_process_cmd = new storage_cmd_t;
	kill_storage_process_cmd -> event_type = SHUTDOWN_STORAGE;
	kill_storage_process_cmd -> sync_semaphore = xSemaphoreCreateBinary();
	
	xQueueSend(*storage_event_queue, &kill_storage_process_cmd, pdMS_TO_TICKS(10000));
	xSemaphoreTake(kill_storage_process_cmd -> sync_semaphore, pdMS_TO_TICKS(30000));
	vSemaphoreDelete(kill_storage_process_cmd -> sync_semaphore);
	
	delete kill_storage_process_cmd;
}