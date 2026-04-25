/*
 * UploaderTypes.hpp
 *
 *  Created on: 19 мар. 2026 г.
 *      Author: Kirill
 */

#ifndef MAIN_UPLOADER_UPLOADERTYPES_HPP_
#define MAIN_UPLOADER_UPLOADERTYPES_HPP_

#include "SensorTypes.hpp"
#include <cstdint>

enum uploader_event_type
{
	SHUTDOWN_UPLOAD = 0,				// Приготовить все к удалению
	
	START_GLOBAL_UPLOAD = 1,			// Запуск выгрузки
	STOP_GLOBAL_UPLOAD = 2,				// Приостановка выгрузки
	FIND_UPLOAD_ADDR = 3,				// Поиск адреса, с которого начинать загрузку
	UPLOAD_BY_ADDR = 4,					// Загрузка по адресу такому-то
};

struct uploader_cmd_t
{
	uploader_event_type event_type;
	uint32_t addr = UINT32_MAX;
	SemaphoreHandle_t sync_semaphore = NULL;
};

#endif /* MAIN_UPLOADER_UPLOADERTYPES_HPP_ */
