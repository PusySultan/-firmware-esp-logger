/*
 * SensorMenager.cpp
 *
 *  Created on: 16 мар. 2026 г.
 *      Author: Kirill
 */

#include "DateTime.hpp"
#include "DustSensor.hpp"
#include "ISensor.hpp"
#include "SensorTypes.hpp"
#include "TermoSensor.hpp"
#include "convertFunc.hpp"
#include "StorageTypes.hpp"
#include "SensorMenager.hpp"
#include "DateTimeSensor.hpp"
#include "memory_w25q_const.hpp"

QueueHandle_t* SensorMenager :: sensor_event_queue = nullptr;
QueueHandle_t* SensorMenager :: storage_event_queue = nullptr;

SensorMenager::SensorMenager ()
{
	fillFunctionMap();
}

SensorMenager::~SensorMenager ()
{
  // TODO Auto-generated destructor stub
}

void SensorMenager :: overrideInternalQueue(QueueHandle_t *queue)
{
	sensor_event_queue = queue;
}

void SensorMenager :: overrideStorageQueue(QueueHandle_t *queue)
{
	storage_event_queue = queue;
}

void SensorMenager :: eventProcessor(sensor_cmd_t* cmd)
{
	if(this -> KILL_PROSESS_FLAG)
	{
		printf("KILL PROCESS FLAG is create, (SensorMenager.eventProcessors)\n");
		delete cmd;
	}
	
	if(!cmd)
	{
		printf("ERROR cmd is NULL, (SensorMenager.eventProcessors)\n");
	}
	
	if(cmd -> event_type < 0 || !this -> eventProcessors.contains(cmd -> event_type))
	{
		printf("ERROR cmd event type is BAD, (SensorMenager.eventProcessors)\n");
		delete cmd;
		return;
	}
	
	this -> eventProcessors[cmd -> event_type](cmd);
	
}

// переодический опрос датчиков
void SensorMenager :: timers_Callback(TimerHandle_t xTimer)
{	
	if(!*sensor_event_queue) return; // Если нет очереди для данных
	
	SensorsID id = static_cast<SensorsID>(
		reinterpret_cast<uintptr_t>(pvTimerGetTimerID(xTimer)));
	
	sensor_cmd_t* sensor_cmd = new sensor_cmd_t;
	sensor_cmd  -> sensor_id = id;
	sensor_cmd -> event_type = SENSOR_GET_VALUE;
	sensor_cmd -> sync_semaphore = NULL;
	
	xQueueSend(*sensor_event_queue, &sensor_cmd , 0);	// Отправляем данные в очередь
}

void SensorMenager :: fillFunctionMap()
{
	this -> eventProcessors[SHUTDOWN_SENSORS] = [this] (sensor_cmd_t* cmd)
	{
		printf("START KILL PROCESS SENSORS\n");
		// Откланяем все поступающие будущие задачи
		this -> KILL_PROSESS_FLAG = true;
		
		// Сохраняем в EEPROM указатель на крайнюю ячейку записи
		saveAllPointer();
		
		// Остановливаем и удаляем таймеры
		this ->  deleteAllTimers();
		this -> timersMap.clear();
		
		// Удаляем все датчики и их таймеры
		this -> sensorsMap.clear();
		
		// Не удалеям команду, она с подтверждением
		if(cmd -> sync_semaphore == NULL) {
			delete cmd;
			return;
		}
		
		xSemaphoreGive(cmd -> sync_semaphore);
	};
	
	this -> eventProcessors[SENSOR_CREATE] = [this] (sensor_cmd_t* cmd)
	{
		// Создаем имя для таймера
		char name[16];
    	snprintf(name, sizeof(name), "Timer%d", cmd -> sensor_id);
    	
    	// Запрашиваем интервал опроса для датчика
    	uint32_t interval = getSensorIntervalById(cmd -> sensor_id);
		printf("Create restart timer %s, for %" PRIu32 " seconds (eventProcessors[SENSOR_CREATE]) \n", name, interval);


		// Создаем датчик и устанавливаем интервал его опроса
		createSensor(cmd -> sensor_id);
		this -> sensorsMap[cmd -> sensor_id] -> setInterval(interval);

		this -> timersMap[cmd -> sensor_id] = xTimerCreate(
	                name,
	        pdMS_TO_TICKS(1 + sensorsMap[cmd -> sensor_id] -> getInterval() * 1000),
	        		pdTRUE,
	        		  (void*)sensorsMap[cmd -> sensor_id] -> getSensorID(),
	         timers_Callback);

	    xTimerStart(this -> timersMap[cmd -> sensor_id], 0);
	    
	    if(cmd -> sync_semaphore == NULL) {
			delete cmd;
			return;
		}
		
		xSemaphoreGive(cmd -> sync_semaphore);
	};
	

	this -> eventProcessors[SENSOR_GET_VALUE] = [this] (sensor_cmd_t* cmd)
	{
		// Получаем значение
		uint8_t convertBuffer[4] = {};
		uint32_t value = this -> sensorsMap[cmd -> sensor_id] -> getData();
		writeU32LE(convertBuffer, value);
		
		// Адрес куда записать
		uint32_t pionter_addr =  this -> sensorsMap[cmd -> sensor_id] -> getPointer();
		
		// Получаем дату и время
		char buffer[32];
		DateTimeSensor::getInstance().ds1302_getDateTime(&this -> dt);
		dt.toString(buffer);
		
		printf("get val %" PRIu32" by sensor %i, by time: %s, save by addres: %" PRIu32 "\n", 
					value, static_cast<int>(cmd -> sensor_id), buffer, pionter_addr);

		// Заполняем данные
		block_data bd0;
		memcpy(bd0.data, convertBuffer, 4);
		bd0.length = 4;
		bd0.addr   = pionter_addr;
		
		// Заполняем время
		block_data bd1;
		memcpy(bd1.data, &this -> dt, sizeof(dt));
		bd1.length =  sizeof(dt);
		bd1.addr   = pionter_addr + bd0.length;
		
		// Заполняем флаг
		block_data bd2;
		bd2.data[0] = 0;
		bd2.length  = 1;
		bd2.addr    = pionter_addr + FLAG_DISTANCE;
		
		// Создаем и заполняем комманду
		storage_cmd_t* storage_cmd = new storage_cmd_t;
		
		storage_cmd -> event_type = WRITE_BY_TRANS;
		storage_cmd -> sync_semaphore       = NULL;
		storage_cmd -> data_size  = 3;
		storage_cmd -> sectorAddr = pionter_addr;
		
		storage_cmd -> data[0]    = bd0;	// data
		storage_cmd -> data[1]    = bd1;	// dateTime
		storage_cmd -> data[2]    = bd2;	// flag
		
		xQueueSend(*storage_event_queue, &storage_cmd, 0);
		
		if(cmd -> sync_semaphore == NULL) {
			delete cmd;
			return;
		}
		
		xSemaphoreGive(cmd -> sync_semaphore);
	};
	
	this -> eventProcessors[SENSOR_UPDATE_SETTINGS] = [this] (sensor_cmd_t* cmd)
	{		
    	storage_cmd_t* storage_cmd = new storage_cmd_t;
    	storage_cmd -> event_type = READ_DATA;
    	storage_cmd -> sync_semaphore = xSemaphoreCreateBinary();
    	storage_cmd -> data_size = 1;
    	storage_cmd -> sectorAddr = this -> settingAddresMap[cmd -> sensor_id];
    	
    	storage_cmd -> data[0].length = 4;
    	storage_cmd -> data[0].addr = this -> settingAddresMap[cmd -> sensor_id];
		
		xQueueSend(*storage_event_queue, &storage_cmd, portMAX_DELAY);
    	xSemaphoreTake(storage_cmd -> sync_semaphore, portMAX_DELAY);
		vSemaphoreDelete(storage_cmd -> sync_semaphore);	
		
		this -> sensorsMap[cmd -> sensor_id] -> setInterval(readU32LE(storage_cmd -> data[0].data));
		
		xTimerStop(this -> timersMap[cmd -> sensor_id], 0);
		xTimerChangePeriod(this -> timersMap[cmd -> sensor_id], this -> sensorsMap[cmd -> sensor_id] -> getInterval()* 1000, 0);
		xTimerStart(this -> timersMap[cmd -> sensor_id], 0);
		
		if(cmd -> sync_semaphore == NULL) {
			delete cmd;
			return;
		}
		
		xSemaphoreGive(cmd -> sync_semaphore);
	};
}

void SensorMenager :: createSensor(SensorsID id)
{
	if(id <= 3)
	{
		printf("create new temp sensor: %i (SensorMenager.createSensor)\n", static_cast<int>(id));
		this -> sensorsMap[id] = new TermoSensor(id);
		return;
	}
	
	if(id == 4)
	{
		// deprecated
		//this -> sensorsMap[id] = new ChipTermoSensor(id);
		return;
	}
	
	if(id > 4 && id < 7)
	{
		printf("create new dust sensor: %i (SensorMenager.createSensor)\n", static_cast<int>(id));
		this -> sensorsMap[id] = new DustSensor(id);
	}
}

uint32_t SensorMenager :: getSensorIntervalById(SensorsID id)
{
	 storage_cmd_t* cmd = new storage_cmd_t;
	 
	 cmd -> event_type = READ_DATA;
     cmd -> sync_semaphore = xSemaphoreCreateBinary();
     cmd -> data_size = 1;
     cmd -> sectorAddr = settingAddresMap[id];
     
     cmd -> data[0].length = 4;
     cmd -> data[0].addr = settingAddresMap[id];
		
    xQueueSend(*storage_event_queue, &cmd, portMAX_DELAY);
    xSemaphoreTake(cmd -> sync_semaphore, portMAX_DELAY);
    
	uint32_t result = readU32LE(cmd -> data[0].data);
	if(result == UINT32_MAX || result < 5)
	{
		uint8_t buffer[4] = {};
		global_settings_t::getInstance().getDefaultSettingByAddr(settingAddresMap[id], buffer);
		
		result = readU32LE(buffer);
	}
	
	
	vSemaphoreDelete(cmd -> sync_semaphore);	
	delete cmd;
	
	return result;
}

void SensorMenager :: saveAllPointer()
{
	std::map<SensorsID, ISensor*>::iterator it = sensorsMap.begin();
	
	while(it != sensorsMap.end())
	{
		this -> sensorsMap[it -> first] -> savePointer();
	}	
}

void  SensorMenager :: deleteAllTimers()
{
	std::map<SensorsID, TimerHandle_t>::iterator it = timersMap.begin();
	
	while(it != timersMap.end())
	{
		xTimerStop(this -> timersMap[it -> first], 10);
		xTimerDelete(this -> timersMap[it -> first], 10);
	}	
}


