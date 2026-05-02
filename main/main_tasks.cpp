/*
 * main_tasks.cpp
 *
 *  Created on: 25 апр. 2026 г.
 *      Author: Kirill
 */


#include "main.hpp"

void createTasks()
{
		xTaskCreate(network_task,
			    "Network task",
			    8192,
			    NULL,
			    8,
			    &networkTaskHundle);
			    
	xTaskCreate(storage_task,
			    "Storage task",
			    4096,
			    NULL,
			    8,
			    &storageTaskHundle);
			    
	xTaskCreate(sensors_task,
			    "Sensors task",
			    4096,
			    NULL,
			    8,
			    &sensorsTaskHundle);
			    
	xTaskCreate(upload_task,
			    "Upload task",
			    4096,
			    NULL,
			    8,
			    &uploadTaskHundle);
			    
	xTaskCreate(create_task,
			    "Create task",
			    4096,
			    NULL,
			    8,
			    &createTaskHundle);
			    
	xTaskCreate(desirializer_task,
			    "Deserialize task",
			    4096,
			    NULL,
			    8,
			    &serverMsgProcessorTaskHundle);
			    
	xTaskCreate(event_task,
			    "Event task",
			    4096,
			    NULL,
			    8,
			    &eventTaskHundle);
}

void network_task(void *pvParameters)
{	
	network_cmd_t* cmd;
	
	while (true)
	{
		if(xQueueReceive(network_event_queue, &cmd, portMAX_DELAY) == pdTRUE)
		{
			if(!network) {
				delete cmd;
				continue;
			}
				
			network -> eventProcessor(cmd);
			// delete cmd;
		}
	}
}

void storage_task(void *pvParameters)
{
	storage_cmd_t* cmd;
	
	while (true)
	{
		if(xQueueReceive(storage_event_queue, &cmd, portMAX_DELAY) == pdTRUE)
		{
			if(!storage) {
				delete cmd;
				continue;
			}
			
			storage -> eventProcessor(cmd);
		}
	}
}

void sensors_task(void *pvParameters)
{
	sensor_cmd_t* cmd;
	
	while(true)
	{
		if(xQueueReceive(sensor_event_queue, &cmd, portMAX_DELAY) == pdTRUE)
		{
			if(!sensor) {
				delete cmd;
				continue;
			}
			
			sensor -> eventProcessor(cmd);
		}
	}
}

// Читает данные из локального хранилища и отправляет в задачу составления запроса на сервер
void upload_task(void *pvParameters)
{
	uploader_cmd_t* cmd;
	
	while(true)
	{
		if(xQueueReceive(upload_event_queue, &cmd, portMAX_DELAY) == pdTRUE)
		{
			if(!uploader) {
				delete cmd;
				continue;
			}
			
			uploader -> eventProcessor(cmd);
		}
	}
}

// Читает данные из локального хранилища и отправляет на сервер
void create_task(void *pvParameters)
{
	create_cmd_t* cmd;
	
	while(true)
	{
		if(xQueueReceive(create_event_queue, &cmd, portMAX_DELAY) == pdTRUE)
		{
			if(!creater){
				 delete cmd;
				 continue;
			}
			
			creater -> eventProcessor(cmd);
		}
	}
}

// Десириализует входные данные от сервера
void desirializer_task(void *pvParameters)
{
	msg_server_cmd* cmd;
	
	while(true)
	{
		if(xQueueReceive(serverMsgProcessor_event_queue, &cmd, portMAX_DELAY) == pdTRUE)
		{
			if(!server_msg_processor) {
				delete cmd;
				continue;
			}
			
			server_msg_processor -> internalProcessor(cmd);
		}
	}
}

void network_task(void *pvParameters)
{
	notif_cmd_t* cmd;

	while (true)
	{
		if(xQueueReceive(notif_event_queue, &cmd, portMAX_DELAY) == pdTRUE)
		{
			if(!myNotif) {
				myNotif ->notifProcessor(cmd);
			}
		}
	}
	
}

void event_task(void *pvParameters)
{
	event_cmd_t* cmd;
	
	while(true)
	{
		if(xQueueReceive(event_queue, &cmd, portMAX_DELAY) == pdTRUE)
		{
			if(!eventMenager) {
				delete cmd;
				continue;
			}
			
			eventMenager -> eventProcessor(cmd);

			if(eventMenager -> getKillStatus()) {
				clearQueue();
				byby();
			}
		}
	}
}