/*
 * SensorTypes.hpp
 *
 *  Created on: 16 мар. 2026 г.
 *      Author: Kirill
 */

#ifndef MAIN_SENSOR_SENSORTYPES_HPP_
#define MAIN_SENSOR_SENSORTYPES_HPP_

#include "freertos/FreeRTOS.h"

enum SensorsID
{
	TEMP_SENSOR_1_ID = 1,
	TEMP_SENSOR_2_ID = 2,
	TEMP_SENSOR_3_ID = 3,
	TEMP_SENSOR_C_ID = 4,
	
	DUST_SENSOR_1_ID = 5,
	DUST_SENSOR_2_ID = 6,
	
	CASE_OPEN_SENSOR_ID = 7,
	
	UNKNOWN_SENSOR = 8
};

enum sensor_event_type_t
{
	SHUTDOWN_SENSORS = 0,				// Остановить задачу
	
	SENSOR_CREATE = 1,
	SENSOR_GET_VALUE = 2,		// запрос данных от датчика
	SENSOR_UPDATE_SETTINGS = 3,	// запросить новые настройки	
};

struct sensor_cmd_t
{
	SensorsID sensor_id;
	sensor_event_type_t event_type;
	SemaphoreHandle_t sync_semaphore = NULL;
};



#endif /* MAIN_SENSOR_SENSORTYPES_HPP_ */
