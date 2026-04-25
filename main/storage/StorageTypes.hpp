/*
 * StorageTypes.hpp
 *
 *  Created on: 14 мар. 2026 г.
 *      Author: Kirill
 */

#ifndef MAIN_STORAGE_STORAGETYPES_HPP_
#define MAIN_STORAGE_STORAGETYPES_HPP_

#include <cstdint>
#include "freertos/FreeRTOS.h"

enum storage_event_type_t
{
	SHUTDOWN_STORAGE = 0,
	
	READ_DATA      = 1,
	WRITE_DATA     = 2,		// Записыываются по разными секторам
	WRITE_BY_TRANS = 3, 	// Записываются внурь 1 сектора
	ERASE_DATA	   = 4,
	SETT_FLAG_SEND = 5,
	
	UPDATE_FROM_SERVER = 9,
};

struct block_data
{
    uint8_t data[65];      // данные 0 - 63  + '\0'
    uint8_t length;        // реальная длина (≤64)
    uint32_t addr;
};

struct storage_cmd_t
{
	storage_event_type_t event_type;		 // Что сделать
	SemaphoreHandle_t sync_semaphore = NULL; // Ждать ли результат
	block_data data[20];					 // блоки данны
	uint8_t data_size = 1;					 // Сколько блоков данных
	uint32_t sectorAddr;					 // Адрес сектора
};

#endif /* MAIN_STORAGE_STORAGETYPES_HPP_ */
