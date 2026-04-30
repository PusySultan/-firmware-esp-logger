/*
 * Storage.cpp
 *
 *  Created on: 14 мар. 2026 г.
 *      Author: Kirill
 */

#include "Storage.hpp"
#include "addresFunc.hpp"
#include "UploaderTypes.hpp"
#include "global_settings_t.hpp"
#include "MessageCreaterTypes.hpp"
#include "memory_w25q_const.hpp"

QueueHandle_t* Storage :: storage_event_queue = nullptr;
QueueHandle_t* Storage :: upload_event_queue = nullptr;
QueueHandle_t* Storage :: msg_createre_event_queue = nullptr;

Storage::Storage ()
{
	this -> memory = new W25Q128();
	
	//fillDefaultSetting();
	fillFunctionMap();
}

Storage::~Storage ()
{
  // TODO Auto-generated destructor stub
}

esp_err_t Storage :: eventProcessor(storage_cmd_t* cmd)
{
	if(!cmd)
	{
		printf("error in Storage.eventProcessor cmd in NULL\n");
		
		delete cmd;
		return ESP_ERR_INVALID_ARG;
	}
	
	if(!cmd -> event_type || !functionMap.contains(cmd -> event_type))
	{
		printf("error in Storage.eventProcessor has not func with id: %d\n", cmd -> event_type);
		
		delete cmd;
		return ESP_ERR_MEMPROT_BASE;
	}
	
	xSemaphoreTake(this -> memory -> semaphore, portMAX_DELAY);
	this -> functionMap[cmd -> event_type](cmd);
	xSemaphoreGive(this -> memory -> semaphore);
	
	return ESP_OK;
}
	   
void Storage :: overrideInternalQueue(QueueHandle_t* queue)
{
	storage_event_queue =  queue;
}

void Storage :: overrideUploadQueue(QueueHandle_t *queue)
{
	upload_event_queue = queue;
}

void Storage :: overrideMsgCreaterQueue(QueueHandle_t *queue)
{
	msg_createre_event_queue = queue;
}

void Storage :: fillFunctionMap()
{
	this -> functionMap[READ_DATA] = [this] (storage_cmd_t* cmd)
	{
		for (uint8_t i = 0; i < cmd -> data_size; i++)
		{
			this -> memory -> readByAddr(
				cmd -> data[i].data, 
				cmd -> data[i].addr, 
				cmd -> data[i].length);
		}
		
		xSemaphoreGive(cmd -> sync_semaphore);
	};
	
	this -> functionMap[WRITE_BY_TRANS] = [this] (storage_cmd_t* cmd)
	{
		this -> memory -> writeTransaction(
			cmd -> data, 
			cmd -> data_size, 
			cmd -> sectorAddr);
		
		uploader_cmd_t* cmd_upload = new uploader_cmd_t;
		cmd_upload -> event_type = UPLOAD_BY_ADDR;
		cmd_upload -> addr = cmd -> data[0].addr;
	
		xQueueSend(*upload_event_queue, &cmd_upload, 0);
	
		delete cmd;
	};
	
	this -> functionMap[WRITE_DATA] = [this] (storage_cmd_t* cmd)
	{
		for (uint8_t i = 0; i < cmd -> data_size; i++)
		{
			this -> memory -> writeByAddr(
			cmd -> data[i].data, 
			cmd -> data[i].addr, 
		    cmd -> data[i].length);	
		}
		
		delete cmd;
	};
	
	this -> functionMap[UPDATE_FROM_SERVER] = [] (storage_cmd_t* cmd)
	{
		creater_block cb0 = {
			.byte_count = sizeof(FREQ_TEMP_SENSOR_1)
		};
		memcpy(cb0.data, FREQ_TEMP_SENSOR_1, cb0.byte_count);
		
		creater_block cb1 = {
			.byte_count = sizeof(FREQ_TEMP_SENSOR_2)
		};
		memcpy(cb1.data, FREQ_TEMP_SENSOR_2, cb1.byte_count);
		
		creater_block cb2 = {
			.byte_count = sizeof(FREQ_TEMP_SENSOR_3)
		};
		memcpy(cb2.data, FREQ_TEMP_SENSOR_3, cb2.byte_count);
		
		creater_block cb3 = {
			.byte_count = sizeof(FREQ_TEMP_SENSOR_C)
		};
		memcpy(cb3.data, FREQ_TEMP_SENSOR_C, cb3.byte_count);
		
		creater_block cb4 = {
			.byte_count = sizeof(FREQ_DUST_SENSOR_1)
		};
		memcpy(cb4.data, FREQ_DUST_SENSOR_1, cb4.byte_count);
		
		creater_block cb5 = {
			.byte_count = sizeof(FREQ_DUST_SENSOR_2)
		};
		memcpy(cb5.data, FREQ_DUST_SENSOR_2, cb5.byte_count);
		
		creater_block cb6 = {
			.byte_count = sizeof(WIFI_NAME)
		};
		memcpy(cb6.data, WIFI_NAME, cb6.byte_count);
		
		creater_block cb7 = {
			.byte_count = sizeof(WIFI_PASS)
		};
		memcpy(cb7.data, WIFI_PASS, cb7.byte_count);
		
		creater_block cb8 = {
			.byte_count = sizeof(DATE_TIME)
		};
		memcpy(cb8.data, DATE_TIME, cb8.byte_count);
		
		create_cmd_t* create_cmd = new create_cmd_t;
		
		create_cmd -> collection = SETTING;
		create_cmd -> cmd_type = CREATE;
		create_cmd -> block_length = 9;
		
		create_cmd -> createrBlock[0] = cb0; 
		create_cmd -> createrBlock[1] = cb1;
		create_cmd -> createrBlock[2] = cb2;
		create_cmd -> createrBlock[3] = cb3;
		create_cmd -> createrBlock[4] = cb4;
		create_cmd -> createrBlock[5] = cb5;
		create_cmd -> createrBlock[6] = cb6;
		create_cmd -> createrBlock[7] = cb7;
		create_cmd -> createrBlock[8] = cb8;
		
		create_cmd -> sync_semaphore = NULL;
		
		xQueueSend(*msg_createre_event_queue, &create_cmd, 0);
	};
	
	
	this -> functionMap[SETT_FLAG_SEND] = [this] (storage_cmd_t* cmd)
	{
		uint8_t flag = 1;
		
		this -> memory -> writeByAddr(
			&flag, 
			cmd -> data[0].addr + FLAG_DISTANCE,
		  1
		);
		
		delete cmd;	
	};
}