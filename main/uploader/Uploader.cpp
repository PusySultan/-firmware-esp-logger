/*
 * Uploader.cpp
 *
 *  Created on: 19 мар. 2026 г.
 *      Author: Kirill
 */

#include "Uploader.hpp"
#include "DateTime.hpp"
#include "SensorTypes.hpp"
#include "UploaderTypes.hpp"
#include "addresFunc.hpp"
#include "convertFunc.hpp"
#include "StorageTypes.hpp"
#include "MessageCreaterTypes.hpp"
#include "memory_w25q_const.hpp"
#include <cstdio>

QueueHandle_t* Uploader :: upload_event_queue = nullptr;
QueueHandle_t* Uploader :: storage_event_queue = nullptr;
QueueHandle_t* Uploader :: creater_event_queue = nullptr;

Uploader :: Uploader ()
{
	fillFunctionMap();
}

Uploader :: ~Uploader ()
{
}

void Uploader :: overrideInternalQueue(QueueHandle_t *queue)
{
	upload_event_queue = queue;	
}

void Uploader :: overrideStorageQueue(QueueHandle_t *queue)
{
	storage_event_queue = queue;
}

void Uploader :: overrideCreaterQueue(QueueHandle_t *queue)
{
	creater_event_queue = queue;
}

void Uploader:: eventProcessor(uploader_cmd_t* cmd)
{
	if(this -> KILL_PROSESS_FLAG) {
		printf("KILL_PROSESS_FLAG is created (Uploader.eventProcessor)\n");
		return;
	}
	
	if(STOP_UPOLAD){
		printf("STOP GLOBAL UPOLAD is created (Uploader.eventProcessor)\n");
		return;
	}
	
	if(!cmd) {
		printf("error cmd in NULL (Uploader.eventProcessor)\n");
		return;
	}
	
	if(cmd -> event_type < 0) {
		printf("error incorrect cmd type (Uploader.eventProcessor)\n");
		return;
	}
	
	if(!functionMap.contains(cmd -> event_type)) {
		printf("error has not func with id: %d\n (Uploader.eventProcessor)", cmd -> event_type);
		return;
	}
	
	// загружаем данные только когда, мы уже загрузили старые
	this -> functionMap[cmd -> event_type](cmd);
}

void Uploader :: fillFunctionMap()
{
	functionMap[SHUTDOWN_UPLOAD] = [this] (uploader_cmd_t* cmd) 
	{
		// Выставляем флаг, и очищаем очередь
		STOP_UPOLAD = true;
		KILL_PROSESS_FLAG = true;
		
		xQueueReset(*upload_event_queue);
		
		if(cmd -> sync_semaphore == NULL)
		{
			delete cmd;
			return;
		}
		
		xSemaphoreGive(cmd -> sync_semaphore);
	};
	
	functionMap[START_GLOBAL_UPLOAD] = [this] (uploader_cmd_t* cmd) 
	{
		// Запуск из NETWORK
		printf("START GLOBAL UPLOAD (Uploader.functionMap[START_GLOBAL_UPLOAD])\n");
		
		if(!ADDR_FINDED) {
			printf("ADDR is not found (Uploader.functionMap[START_GLOBAL_UPLOAD])\n");
			return;
		}
		
		STOP_UPOLAD = false;
		
		// Начинаем по очереди, перебирать данные
		uint8_t data[4];
		uint8_t dateTime[32];
		uint8_t stableCycles = 0;
		SensorsID currentId = TEMP_SENSOR_1_ID;
		
		do
		{	
			printf("new iteartion\n");
			uploadDataByAddr(uploadAddr[currentId], data, dateTime);

			if(!arrayContainsTrush(data, 4))
			{
				printf("data is correct by id %d\n", currentId);
				if(STOP_UPOLAD) {
					printf("BREAK FROM UPLOADER DATA\n");
					break;
				}
				
				this -> uploadStateById[currentId] = false;										
				this -> sendToServer(currentId, data, dateTime);
				this -> getNextAddr(currentId);
			
				vTaskDelay(pdMS_TO_TICKS(1000));
			} else {
				printf("data is trash by id %d\n", currentId);
				// помечаем данные по данному ID загруженными
				uploadStateById[currentId] = true;
				currentId = getNextId(currentId);				
			}

			if(checkAllStates()) {
				reesteAllStates();
				stableCycles++;
			}

			if(stableCycles >= 3) {
				// Делаем несколько проходов
				printf("BREAK FROM UPLOADER DATA\n");
				break;
			}

			vTaskDelay(pdMS_TO_TICKS(1000));
		}
		while(!STOP_UPOLAD);
		
		OLD_DATA_UPLOAD = true;
		printf("OLD DATA UPLOADED (Uploader.functionMap[START_GLOBAL_UPLOAD])\n");
		
		if(cmd -> sync_semaphore == NULL) {
			delete cmd;
			return;
		}
		
		xSemaphoreGive(cmd -> sync_semaphore);
	};
	
	functionMap[STOP_GLOBAL_UPLOAD] = [this] (uploader_cmd_t* cmd) 
	{
		STOP_UPOLAD = true;
		
		if(cmd -> sync_semaphore == NULL) {
			delete cmd;
			return;
		}
		
		xSemaphoreGive(cmd -> sync_semaphore);
	};
	
	functionMap[FIND_UPLOAD_ADDR] = [this] (uploader_cmd_t* cmd) 
	{		
		std::map<SensorsID, uint32_t>::iterator it = uploadAddr.begin();
		while(it != uploadAddr.end())
		{
    		this -> uploadAddrById(it -> first);
			it++;
		}
		
		ADDR_FINDED = true;
		
		if(cmd -> sync_semaphore == NULL) {
			delete cmd;
			return;
		}
		
		xSemaphoreGive(cmd -> sync_semaphore);
	};
	
	functionMap[UPLOAD_BY_ADDR] = [this](uploader_cmd_t* cmd)
	{	
		if(!OLD_DATA_UPLOAD) return; // Если старые данные не загруженны, новые не трогаем

		if(!storage_event_queue) {
			printf("Does not exist storage queue (functionMap[UPLOAD_BY_ID])\n");
			return;	
		}
		
		if(cmd -> addr ==  UINT32_MAX || cmd -> addr < 1) {
			printf("Incorrect UPLOAD addres (functionMap[UPLOAD_BY_ID])\n");
			return;
		}
		
		if(STOP_UPOLAD) return; 	 // Если вызвана остановка загрузки
				
		printf("UPLOAD BY ADDR\n");
		
		uint8_t data[4];
		uint8_t dateTime[32];
		SensorsID currentId = getSensorIdByAddr(cmd -> addr);
		printf("upload sensor id %d addr: %" PRIu32 "\n", static_cast<uint8_t>(currentId), cmd -> addr);
		
		// Загружаем данные из памяти
		uploadDataByAddr(cmd -> addr, data, dateTime);
		sendToServer(currentId, data, dateTime);
				
		uploadAddr[currentId] += NEXT_SECTOR;
		
		if(cmd -> sync_semaphore == NULL) {
			delete cmd;
			return;
		}
		
		xSemaphoreGive(cmd -> sync_semaphore);
	};
}

bool Uploader :: checkAllStates()
{
	bool state = uploadStateById[TEMP_SENSOR_1_ID];
	std::map<SensorsID, bool>::iterator it = uploadStateById.begin();

	while(it != uploadStateById.end())
	{
		state = state && it -> second;
		++it;
	}

	printf("check all states and return: %i\n", state);
	return state;
	/*
		return uploadStateById[TEMP_SENSOR_1_ID] && uploadStateById[TEMP_SENSOR_2_ID] && uploadStateById[TEMP_SENSOR_3_ID] &&
		   uploadStateById[TEMP_SENSOR_C_ID] && uploadStateById[DUST_SENSOR_1_ID] && uploadStateById[DUST_SENSOR_2_ID] &&
		   uploadStateById[CASE_OPEN_EVENT_ID] && uploadStateById[CASE_CLOSE_EVENT_ID];
	*/
}

void Uploader :: reesteAllStates()
{
	std::map<SensorsID, bool>::iterator it = uploadStateById.begin();

	while(it != uploadStateById.end())
	{
		it -> second = false;
		++it;
	}

	printf("reset all states");
}

// Ищем по 1000 страницам
void Uploader :: uploadAddrById(SensorsID id)
{
    uint32_t leftPage = sector_addr_t[id].first / SECTOR_SIZE;   // переводим байты в номера страниц
    uint32_t rightPage = sector_addr_t[id].second / SECTOR_SIZE;
    uint32_t currentPage = leftPage;
    uint8_t flag = 0;

    while (leftPage < rightPage)
    {
        currentPage = leftPage + (rightPage - leftPage) / 2;
        uint32_t flagAddr = currentPage * SECTOR_SIZE + (SECTOR_SIZE - 1);   // последний байт страницы
        readFlag(flagAddr, &flag);
		
        if (flag == 1)
        {
            leftPage = currentPage + 1;
            continue;
        }
        
        // сектор не отправлен — ищем в левой половине (ближе)
        rightPage = currentPage - 1;
    }
    
    // После выхода currentPage указывает на последнюю проверенную страницу
    uploadAddr[id] = currentPage * SECTOR_SIZE;   // сохраняем адрес начала страницы
    printf("find addr %" PRIu32 " by sensor %" PRIu8 "\n\n", uploadAddr[id], id);
}

void Uploader :: readFlag(uint32_t addr, uint8_t* flag)
{
	if(!storage_event_queue) return;
	
	storage_cmd_t* get_data_from_storage_cmd = new storage_cmd_t;
	
	get_data_from_storage_cmd -> event_type = READ_DATA;
	get_data_from_storage_cmd -> data_size = 1;
	get_data_from_storage_cmd -> sectorAddr = addr;
	get_data_from_storage_cmd -> sync_semaphore = xSemaphoreCreateBinary();
	
	get_data_from_storage_cmd -> data[0].length = 1;
	get_data_from_storage_cmd -> data[0].addr = addr;
		
	xQueueSend(*storage_event_queue, &get_data_from_storage_cmd, portMAX_DELAY);
	xSemaphoreTake(get_data_from_storage_cmd -> sync_semaphore, portMAX_DELAY);
	vSemaphoreDelete(get_data_from_storage_cmd -> sync_semaphore);
	
	// из 1 блока данных копируем 1 байт
	memcpy(flag, get_data_from_storage_cmd -> data[0].data, 1);
	delete get_data_from_storage_cmd;
}

void Uploader :: getNextAddr(SensorsID id)
{
	if((uint8_t)id < 7)
	{
		uploadAddr[id] += NEXT_SECTOR;
	}
}

void Uploader :: uploadDataByAddr(uint32_t addr, uint8_t* data, uint8_t* dt)
{
	if(!storage_event_queue) {
		printf("Dtorage queue is NULL (Uploader.uploadDataByAddr)\n");
		return;
	}
		
	storage_cmd_t* get_data_from_storage_cmd = new storage_cmd_t;
	
	get_data_from_storage_cmd -> event_type = READ_DATA;
	get_data_from_storage_cmd -> sync_semaphore = xSemaphoreCreateBinary();
	get_data_from_storage_cmd -> data_size = 2;
	get_data_from_storage_cmd -> sectorAddr = addr;
	
	get_data_from_storage_cmd -> data[0].length = 4;		// данные (значение, тип события и др)
	get_data_from_storage_cmd -> data[0].addr = addr;
	
	get_data_from_storage_cmd -> data[1].length = sizeof(DateTime);
	get_data_from_storage_cmd -> data[1].addr = addr + 4;
	
	xQueueSend(*storage_event_queue, &get_data_from_storage_cmd, portMAX_DELAY);
	xSemaphoreTake(get_data_from_storage_cmd -> sync_semaphore, portMAX_DELAY);
	vSemaphoreDelete(get_data_from_storage_cmd -> sync_semaphore);
	
	memcpy(data, get_data_from_storage_cmd -> data[0].data, get_data_from_storage_cmd -> data[0].length);
	memcpy(dt,   get_data_from_storage_cmd -> data[1].data, get_data_from_storage_cmd -> data[1].length);
	
	delete get_data_from_storage_cmd;
}

void Uploader :: sendToServer(SensorsID id, uint8_t* data, uint8_t* dt)
{
	create_cmd_t* create_cmd = new create_cmd_t;
	
	create_cmd -> cmd_type = CREATE;
	create_cmd -> collection = getCmdCollection(id);
	create_cmd -> dateTime = DateTime::fromBytesStatic(dt);	
	create_cmd -> block_length = 2;
	
	memcpy(create_cmd -> createrBlock[0].data, data, 4);
	create_cmd -> createrBlock[0].byte_count = 4;
	
	writeU32LE(create_cmd -> createrBlock[1].data, id);
	create_cmd -> createrBlock[1].byte_count = sizeof(id);
	
	create_cmd -> sync_semaphore = NULL;
	
	xQueueSend(*creater_event_queue, &create_cmd, 0);	
		
	resetFlagById(id);
}

SensorsID Uploader :: getNextId(SensorsID id)
{
	uint8_t next = (uint8_t)id + 1;

	if(next > CASE_CLOSE_EVENT_ID) {
		next = TEMP_SENSOR_1_ID;
	}

	return static_cast<SensorsID>(next);
}

void Uploader :: resetFlagById(SensorsID id)
{
	if(!storage_event_queue) return;
	
	// Для данных снимаем флаг отправки (пока не делаем)
	if(id < 7)
	{
		/*
		storage_cmd_t* set_flag_cmd = new storage_cmd_t;
	
		set_flag_cmd -> event_type = SETT_FLAG_SEND;
		set_flag_cmd -> data[0].addr = uploadAddr[id];

		// команда удаляется в обработчике
		xQueueSend(*storage_event_queue, &set_flag_cmd, 0);
		return;
		*/
	}

	// Для событий стираем запись
	storage_cmd_t* delete_event_cmd = new storage_cmd_t;
	
	delete_event_cmd -> event_type = ERASE_DATA;
	delete_event_cmd -> sectorAddr = uploadAddr[id];
	delete_event_cmd -> sync_semaphore = xSemaphoreCreateBinary(); 

	xQueueSend(*storage_event_queue, &delete_event_cmd, portMAX_DELAY);
	xSemaphoreTake(delete_event_cmd -> sync_semaphore, portMAX_DELAY);
	vSemaphoreDelete(delete_event_cmd -> sync_semaphore);	// 419

	delete delete_event_cmd;
}

collection_t Uploader :: getCmdCollection(SensorsID id)
{
	if (id == TEMP_SENSOR_1_ID || id == TEMP_SENSOR_2_ID || 
		id == TEMP_SENSOR_3_ID || id == TEMP_SENSOR_C_ID)
	{
		return DEVICE_TEMP;
	}
	
	if (id == DUST_SENSOR_1_ID || id == DUST_SENSOR_2_ID)
	{
		return DEVICE_DUST;

	};

	if(id == CASE_CLOSE_EVENT_ID  || id == CASE_OPEN_EVENT_ID)
	{
		return DEVICE_EVENT;
	}

	return UNKNOW_COLLECTION;	
}




