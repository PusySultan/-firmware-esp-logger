/*
 * SettingCollection.cpp
 *
 *  Created on: 18 апр. 2026 г.
 *      Author: Kirill
 */

#include "DateTimeSensor.hpp"
#include "global_settings_t.hpp"
#include "PostSettingCollection.hpp"

QueueHandle_t* PostSettingCollection ::  storage_event_queue;

PostSettingCollection :: PostSettingCollection (collection_t collection) 
				   : ICollectionProcessor(collection)
{
	fillProcessorsMap();
}

PostSettingCollection :: ~PostSettingCollection ()
{
	this -> processorsMap.clear();
}

void PostSettingCollection :: overrideStorageQueue(QueueHandle_t* queue)
{
	storage_event_queue = queue;
}

// Принимаем ссылку на объект
void PostSettingCollection :: process(const request_t& request)
{
	cJSON* answer = cJSON_GetObjectItem(request.message, "answer");
    if (cJSON_IsNull(answer))
    {
		printf("ERROR answer is NULL (SettingCollection.process)\n");
        return;
    }
        
    storage_cmd_t* cmd = new storage_cmd_t;
    cmd -> event_type = WRITE_BY_TRANS;
    cmd -> sectorAddr = global_settings_t::WIFI_NAME_ADDR;
    
    cJSON *currentItem = answer -> child;
   	
   	// valuestring - значение
   	// string - имя поля
   	while (currentItem != NULL)
	{	 
		if (currentItem  -> valuestring == NULL)
	    {
			currentItem  = currentItem  -> next;  // переход к следующему полю
	        continue;
	    }
	    
	    std::string key(currentItem -> string);
	    if(!this -> processorsMap.contains(key))
	    {
			printf("dose not exists map entry %s\n", currentItem -> string);
			currentItem  = currentItem  -> next;  // переход к следующему полю
	        continue;
		}
	    
	    this -> processorsMap[key](*cmd, *currentItem);
	    currentItem  = currentItem  -> next;  // переход к следующему полю
	 }
	 
	 xQueueSend(*storage_event_queue, &cmd, 0);
}

void PostSettingCollection :: fillProcessorsMap()
{
	this -> processorsMap[std::string(DATE_TIME)] = [](storage_cmd_t& cmd, cJSON& currentItem)
	{
		DateTime dt = DateTime::fromStringStatic(currentItem.valuestring);
		DateTimeSensor::getInstance().ds1302_setDateTime(&dt);
		DateTimeSensor::getInstance().ds1302_getDateTime(&dt);
				
		char buffer[32];
		dt.toString(buffer);
		printf("DateTime after set: %s\n", buffer);
	};
	
	this -> processorsMap[std::string(FREQ_TEMP_SENSOR_1)] = [](storage_cmd_t& cmd, cJSON& currentItem)
	{
		uint32_t value = (uint32_t)atoi(currentItem.valuestring);
		
		cmd.data[cmd.data_size].addr = global_settings_t::getInstance().getSettingAddrByName(currentItem.string);
		cmd.data[cmd.data_size].length = sizeof(uint32_t);
		memcpy(cmd.data[cmd.data_size].data, &value, sizeof(value));
			
		cmd.data_size += 1;
	};
	
	this -> processorsMap[std::string(FREQ_TEMP_SENSOR_2)] = [](storage_cmd_t& cmd, cJSON& currentItem)
	{
		uint32_t value = (uint32_t)atoi(currentItem.valuestring);
		
		cmd.data[cmd.data_size].addr = global_settings_t::getInstance().getSettingAddrByName(currentItem.string);
		cmd.data[cmd.data_size].length = sizeof(uint32_t);
		memcpy(cmd.data[cmd.data_size].data, &value, sizeof(value));
			
		cmd.data_size += 1;
	};
	
	this -> processorsMap[std::string(FREQ_TEMP_SENSOR_3)] = [](storage_cmd_t& cmd, cJSON& currentItem)
	{
		uint32_t value = (uint32_t)atoi(currentItem.valuestring);
		
		cmd.data[cmd.data_size].addr = global_settings_t::getInstance().getSettingAddrByName(currentItem.string);
		cmd.data[cmd.data_size].length = sizeof(uint32_t);
		memcpy(cmd.data[cmd.data_size].data, &value, sizeof(value));
			
		cmd.data_size += 1;
	};
	
	this -> processorsMap[std::string(FREQ_TEMP_SENSOR_C)] = [](storage_cmd_t& cmd, cJSON& currentItem)
	{uint32_t value = (uint32_t)atoi(currentItem.valuestring);
		
		cmd.data[cmd.data_size].addr = global_settings_t::getInstance().getSettingAddrByName(currentItem.string);
		cmd.data[cmd.data_size].length = sizeof(uint32_t);
		memcpy(cmd.data[cmd.data_size].data, &value, sizeof(value));
			
		cmd.data_size += 1;
	};
	
	this -> processorsMap[std::string(FREQ_DUST_SENSOR_1)] = [](storage_cmd_t& cmd, cJSON& currentItem)
	{
		uint32_t value = (uint32_t)atoi(currentItem.valuestring);
		
		cmd.data[cmd.data_size].addr = global_settings_t::getInstance().getSettingAddrByName(currentItem.string);
		cmd.data[cmd.data_size].length = sizeof(uint32_t);
		memcpy(cmd.data[cmd.data_size].data, &value, sizeof(value));
			
		cmd.data_size += 1;
	};
	
	this -> processorsMap[std::string(FREQ_DUST_SENSOR_2)] = [](storage_cmd_t& cmd, cJSON& currentItem)
	{
		uint32_t value = (uint32_t)atoi(currentItem.valuestring);
		
		cmd.data[cmd.data_size].addr = global_settings_t::getInstance().getSettingAddrByName(currentItem.string);
		cmd.data[cmd.data_size].length = sizeof(uint32_t);
		memcpy(cmd.data[cmd.data_size].data, &value, sizeof(value));
			
		cmd.data_size += 1;
	};
	
	this -> processorsMap[std::string(WIFI_NAME)] = [](storage_cmd_t& cmd, cJSON& currentItem)
	{
		uint8_t str_len = std::strlen(currentItem.valuestring);
		
		cmd.data[cmd.data_size].addr = global_settings_t::getInstance().getSettingAddrByName(currentItem.string);
		cmd.data[cmd.data_size].length = str_len + 1;
		cmd.data[cmd.data_size].data[str_len] = '\0';
		memcpy(cmd.data[cmd.data_size].data, currentItem.valuestring, str_len);
		
		cmd.data_size += 1;
	};
	
	this -> processorsMap[std::string(WIFI_PASS)] = [](storage_cmd_t& cmd, cJSON& currentItem)
	{
		uint8_t str_len = std::strlen(currentItem.valuestring);
		
		cmd.data[cmd.data_size].addr = global_settings_t::getInstance().getSettingAddrByName(currentItem.string);
		cmd.data[cmd.data_size].length = str_len + 1;
		cmd.data[cmd.data_size].data[str_len] = '\0';
		memcpy(cmd.data[cmd.data_size].data, currentItem.valuestring, str_len);
			
		cmd.data_size += 1;
	};
}
