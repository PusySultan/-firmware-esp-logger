/*
 * timeIntervalFunc.cpp
 *
 *  Created on: 6 мая 2026 г.
 *      Author: Kirill
 */

#include "EventMenager.hpp"
#include "EventTypes.hpp"
#include "StorageTypes.hpp"
#include "convertFunc.hpp"
#include "memory_w25q_const.hpp"

// Сохраняет в память данные о включении
// 0 блок - код события (u32)
// 1 блок время события
void EventMenager :: saveEnableEvent()
{
	char buffer[32];
	this -> enebleTime.toString(buffer);
	printf("Start date - time (EventMenager.eventProcessors[DEVICE_EVENT_ON]) %s\n", buffer);	

	storage_cmd_t* stor_cmd = new storage_cmd_t;
	
	stor_cmd -> event_type = WRITE_DATA;
	stor_cmd -> sectorAddr = ADDR_EVENT_DEVICE_ON;
	stor_cmd -> sync_semaphore = NULL;
	stor_cmd -> data_size = 2;

	stor_cmd -> data[0].addr = ADDR_EVENT_DEVICE_ON;
    stor_cmd -> data[0].length = sizeof(DEVICE_EVENT_ON);
    writeU32LE(stor_cmd -> data[0].data, DEVICE_EVENT_ON);
	
    stor_cmd -> data[1].addr = ADDR_EVENT_DEVICE_ON + stor_cmd -> data[0].length;
    stor_cmd -> data[1].length = sizeof(enebleTime);
    memcpy(stor_cmd -> data[1].data, &enebleTime, sizeof(enebleTime));

	xQueueSend(*storage_event_queue, &stor_cmd, 0);	
}

// Сохраняет в память данные о выключении устройства
// 0 блок - код события (u32)
// 1 блок время события
void EventMenager :: saveDisabeEvent()
{
	storage_cmd_t* reg_event_off = new storage_cmd_t;
	
	reg_event_off -> event_type = WRITE_BY_TRANS;
	reg_event_off -> sectorAddr = ADDR_EVENT_DEVICE_OFF;
    reg_event_off -> sync_semaphore = NULL;
	reg_event_off -> data_size = 2;

    reg_event_off -> data[0].addr = ADDR_EVENT_DEVICE_OFF;
	reg_event_off -> data[0].length = sizeof(disableTime);
	memcpy(reg_event_off -> data[0].data, &disableTime, sizeof(disableTime));

	reg_event_off -> data[1].addr = ADDR_EVENT_DEVICE_OFF + reg_event_off -> data[0].length;
	reg_event_off -> data[1].length = sizeof(disableTime);
	memcpy(reg_event_off -> data[1].data, &disableTime, sizeof(disableTime));

	xQueueSendToFront(*storage_event_queue, reg_event_off, pdMS_TO_TICKS(10000));
}

void EventMenager :: saveJobIntervalTime()
{
	uint64_t seconds = disableTime.toSecondsShort() - enebleTime.toSecondsShort();

	storage_cmd_t* reg_event_off = new storage_cmd_t;	
	reg_event_off -> event_type = WRITE_DATA;
	reg_event_off -> sectorAddr =  ADDR_LAST_JOB_INTERVAL;
	reg_event_off -> data_size = 1;
	reg_event_off -> data[0].addr = ADDR_LAST_JOB_INTERVAL;
	reg_event_off -> data[0].length = sizeof(seconds);
	writeU64LE(reg_event_off -> data[0].data, seconds);

	xQueueSendToFront(*storage_event_queue, reg_event_off, pdMS_TO_TICKS(10000));
}

void EventMenager :: saveTotalJobTime()
{
	uint64_t seconds = getTotalJobTime();
	seconds += disableTime.toSecondsShort();

	storage_cmd_t* reg_event_off = new storage_cmd_t;
	
	reg_event_off -> event_type = WRITE_BY_TRANS;
	reg_event_off -> sectorAddr =  ADDR_TOTAL_JOB_TIME;
	reg_event_off -> data_size = 1;
	reg_event_off -> data[0].addr = ADDR_TOTAL_JOB_TIME;
	reg_event_off -> data[0].length = sizeof(seconds);
	writeU64LE(reg_event_off -> data[0].data, seconds);

	xQueueSendToFront(*storage_event_queue, reg_event_off, pdMS_TO_TICKS(10000));
}

uint64_t EventMenager :: getTotalJobTime()
{
	uint64_t seconds = 0;

	storage_cmd_t* storage_cmd = new storage_cmd_t;
		
	storage_cmd -> event_type = READ_DATA;
	storage_cmd -> data_size = 1;
	storage_cmd -> sectorAddr = ADDR_TOTAL_JOB_TIME;
	storage_cmd -> sync_semaphore = xSemaphoreCreateBinary();
	storage_cmd -> data[0].length = sizeof(seconds);
	storage_cmd -> data[0].addr = ADDR_TOTAL_JOB_TIME;

	xQueueSend(*storage_event_queue, &storage_cmd, pdMS_TO_TICKS(10000));
	xSemaphoreTake(storage_cmd -> sync_semaphore, pdMS_TO_TICKS(30000));
	vSemaphoreDelete(storage_cmd -> sync_semaphore);

	if(arrayContainsTrush(storage_cmd -> data[0].data, storage_cmd -> data[0].length)) {
		seconds = readU64LE(storage_cmd -> data[0].data);
	}

	delete storage_cmd;
	return seconds;
}


