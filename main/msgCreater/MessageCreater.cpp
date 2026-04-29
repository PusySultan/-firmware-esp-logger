/*
 * MessageCreater.cpp
 *
 *  Created on: 19 мар. 2026 г.
 *      Author: Kirill
 */

#include "esp_mac.h"
#include "eventFunc.hpp"
#include "convertFunc.hpp"
#include "NetworkTypes.hpp"
#include "MessageCreater.hpp"

QueueHandle_t* MessageCreater :: create_event_queue = nullptr;
QueueHandle_t* MessageCreater :: network_event_queue = nullptr;

MessageCreater::MessageCreater ()
{
	this -> fillReqInfo();
	this -> fillFunctionMap();
}

MessageCreater::~MessageCreater ()
{
  // TODO Auto-generated destructor stub
}

void MessageCreater :: overrideInternalQueue(QueueHandle_t *queue)
{
	create_event_queue = queue;
}

void MessageCreater :: overrideNetworkQueue(QueueHandle_t *queue)
{
	network_event_queue = queue;	
}

void MessageCreater :: eventProcessor(create_cmd_t* cmd)
{
	if(this -> KILL_PROSESS_FLAG) {
		 printf("KILL PROCESS FLAG is created (MessageCreater.EventProcessor)\n"); return;
	}
	
	if(!cmd) {
		printf("Error: cmd is NULL (MessageCreater.EventProcessor)\n");
		return; // 49 строка
	}
	
	if(cmd -> cmd_type == SHUTDOWN_MSG_CREATER) // 52 строка
	{
		printf("KILL process creater (MessageCreater.EventProcessor)\n");
		this -> killProcess();
		
		if(cmd -> sync_semaphore == NULL) {
			delete cmd;
			return;
		}
		
		xSemaphoreGive(cmd -> sync_semaphore);	
		return;	
	}
	
	if(cmd -> collection < 0 || !functionMap.contains(cmd -> collection)) {
		printf("Error: Dose not exist cmd (MessageCreater.EventProcessor)\n");
		return;
	}
	
	functionMap[cmd -> collection](cmd);
}

void MessageCreater :: fillFunctionMap()
{	
	// only push
	functionMap[DEVICE_TEMP]	= [this] (create_cmd_t *cmd) 
	{	
		cJSON* topJSON = cJSON_CreateObject();
		cJSON* message = cJSON_CreateObject();
		
		char dateTimeStr[32];
		cmd -> dateTime.toString(dateTimeStr);
		
		cJSON_AddStringToObject(message, "date", dateTimeStr);
		cJSON_AddNumberToObject(message, "sensorInfo", readU32LE(cmd -> createrBlock[1].data));
		cJSON_AddNumberToObject(message, "val", readU32LE(cmd -> createrBlock[0].data));
		
		cJSON_AddStringToObject(topJSON, "type", "POST");
		cJSON_AddStringToObject(topJSON, "collection", collection_to_string(cmd -> collection));
		cJSON_AddItemToObject(topJSON, "message", message);
		cJSON_AddItemToObject(topJSON, "requesterInfo", cJSON_Duplicate(this -> requesterInfo, true));		

		char* jsonStr = cJSON_Print(topJSON);
		sendToServer(jsonStr);
		
		cJSON_free(jsonStr);
		cJSON_Delete(topJSON);
		
		if(cmd -> sync_semaphore == NULL) {
			delete cmd;
			return;
		}
		
		xSemaphoreGive(cmd -> sync_semaphore);
	};
	
	// only push
	functionMap[DEVICE_DUST] 	= [this] (create_cmd_t *cmd) 
	{
		cJSON* topJSON = cJSON_CreateObject();
		cJSON* message = cJSON_CreateObject();
		
		char dateTimeStr[32];
		cmd -> dateTime.toString(dateTimeStr);
		
		cJSON_AddStringToObject(message, "date", dateTimeStr);
		cJSON_AddNumberToObject(message, "val", readU32LE(cmd -> createrBlock[0].data));
		
		cJSON_AddStringToObject(topJSON, "type", "POST");
		cJSON_AddStringToObject(topJSON, "collection", collection_to_string(cmd -> collection));
		cJSON_AddItemToObject(topJSON, "message", message);
		cJSON_AddItemToObject(topJSON, "requesterInfo", cJSON_Duplicate(this -> requesterInfo, true));
		
		char* jsonStr = cJSON_Print(topJSON);
		sendToServer(jsonStr);
		
		cJSON_free(jsonStr);
		cJSON_Delete(topJSON);
		
		if(cmd -> sync_semaphore == NULL) {
			delete cmd;
			return;
		}
		
		xSemaphoreGive(cmd -> sync_semaphore);
	};
	
	functionMap[DEVICE_EVENT] 	= [this] (create_cmd_t *cmd) 
	{
		cJSON* topJSON = cJSON_CreateObject();
		cJSON* message = cJSON_CreateObject();
		
		char *event = {};
		char dateTimeStr[32];
		cmd -> dateTime.toString(dateTimeStr);
		event =  event_type_to_string((event_type_t)readU32LE(cmd -> createrBlock[0].data));
		
		cmd -> dateTime.toString(dateTimeStr);
		cJSON_AddStringToObject(message, "date", dateTimeStr);
		cJSON_AddStringToObject(message, "val", event);
		
		cJSON_AddStringToObject(topJSON, "type", "POST");
		cJSON_AddStringToObject(topJSON, "collection", collection_to_string(cmd -> collection));
		cJSON_AddItemToObject(topJSON, "message", message);
		cJSON_AddItemToObject(topJSON, "requesterInfo", cJSON_Duplicate(this -> requesterInfo, true));
		
		char* jsonStr = cJSON_Print(topJSON);
		sendToServer(jsonStr);
		
		cJSON_free(jsonStr);
		cJSON_Delete(topJSON);
		
		if(cmd -> sync_semaphore == NULL) {
			delete cmd;
			return;
		}
		
		xSemaphoreGive(cmd -> sync_semaphore);
	};
	
	functionMap[SETTING] 		= [this] (create_cmd_t *cmd) 
	{		
		cJSON* searchParamObj  = cJSON_CreateObject();
		cJSON* searchParamArr  = cJSON_CreateArray();
		
		// search param
		for(uint8_t i = 0; i < cmd -> block_length; i++)
		{
			char settingName[65] = {0};
			memcpy(settingName, cmd -> createrBlock[i].data, cmd -> createrBlock[i].byte_count);
			
			cJSON* searchParamItem = cJSON_CreateObject();
			
			cJSON_AddStringToObject(searchParamItem,"field", "name");
			cJSON_AddStringToObject(searchParamItem,"value", settingName);
			cJSON_AddItemToArray(searchParamArr, searchParamItem);	
		}
		
		cJSON_AddItemToObject(searchParamObj, "param", searchParamArr);
		cJSON_AddStringToObject(searchParamObj,"operator", "equal");
				
		//  --- get param
		cJSON* getParamArr 	= cJSON_CreateArray();		
		cJSON_AddItemToArray(getParamArr, cJSON_CreateString("value"));
		
		// --- top level JSON
		cJSON* topJSON     = cJSON_CreateObject();
		cJSON* message     = cJSON_CreateObject();
		
		cJSON_AddItemToObject(message, "searchParam", searchParamObj);
		cJSON_AddItemToObject(message, "getParam", getParamArr);


		// --- fill request info
		cJSON_AddStringToObject(topJSON, "type", "GET");
		cJSON_AddStringToObject(topJSON, "collection", collection_to_string(cmd -> collection));
		cJSON_AddItemToObject(topJSON, "message", message);
		cJSON_AddItemToObject(topJSON, "requesterInfo", cJSON_Duplicate(this -> requesterInfo, true));
		
		// --- print
		char* jsonStr = cJSON_Print(topJSON);
		sendToServer(jsonStr);
		
		cJSON_free(jsonStr);
		cJSON_Delete(topJSON);
		
		if(cmd -> sync_semaphore == NULL) {
			delete cmd;
			return;
		}
		
		xSemaphoreGive(cmd -> sync_semaphore);
	};

}
	
void MessageCreater :: fillReqInfo()
{
	uint8_t baseMac[6];
    char strMac[18]; // 6 байт
    
	esp_read_mac(baseMac, ESP_MAC_WIFI_STA);
	
	 // Конвертация в HEX: "A1B2C3D4E5F6"
	sprintf(strMac, "%02X:%02X:%02X:%02X:%02X:%02X",
        baseMac[0], baseMac[1], baseMac[2], 
        baseMac[3], baseMac[4], baseMac[5]);
	
	
	this -> requesterInfo = cJSON_CreateObject();
	cJSON_AddStringToObject(this ->  requesterInfo, "entitiesType", "Device");
	cJSON_AddStringToObject(this -> requesterInfo, "guid", strMac);
	
	cJSON* auntInfo = cJSON_CreateObject();
	cJSON_AddStringToObject(auntInfo, "userName", "");
	cJSON_AddStringToObject(auntInfo, "hash", "");
	
	cJSON_AddItemToObject(this -> requesterInfo, "auntInfo", auntInfo); 	
}

void MessageCreater :: sendToServer(char* data)
{
	if (data == nullptr) {
		printf("Error: Data to be sent to the server (MessageCreater.EventProcessor)\n");
		 return;
	};

	network_cmd_t* cmd = new network_cmd_t;
    cmd -> event_type = SEND_MESSAGE;
    
    // Копируем данные с защитой от переполнения
    size_t len = strlen(data);
    if (len >= 1024) len = 1023;
    
    memcpy(cmd -> data, data, len);
    cmd -> data[len] = '\0';  // Null-terminate
	
	xQueueSend(*network_event_queue, &cmd, 0);
}

void MessageCreater :: killProcess()
{
	this -> KILL_PROSESS_FLAG = true;
	
	xQueueReset(*create_event_queue);
	cJSON_Delete(this -> requesterInfo);
	
	functionMap.clear();
}
