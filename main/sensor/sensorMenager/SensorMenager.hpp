/*
 * SensorMenager.hpp
 *
 *  Created on: 16 мар. 2026 г.
 *      Author: Kirill
 */

#ifndef MAIN_SENSOR_SENSORMENAGER_SENSORMENAGER_HPP_
#define MAIN_SENSOR_SENSORMENAGER_SENSORMENAGER_HPP_

#include <map>
#include <functional>
#include "ISensor.hpp"
#include "DateTime.hpp"
#include "freertos/FreeRTOS.h"
#include "global_settings_t.hpp"

class SensorMenager final
{
	using func = std::function<void(sensor_cmd_t*)>;
	
	public:
		SensorMenager ();
	    virtual ~SensorMenager ();
	    void eventProcessor(sensor_cmd_t* cmd);
	    static void overrideInternalQueue(QueueHandle_t* queue);
	    static void overrideStorageQueue(QueueHandle_t* queue);
	    
	private:
		DateTime dt;
		
		// мапа лямбда функций
		std::map<sensor_event_type_t, func> eventProcessors;
			
		// мапа датчиков
		std::map<SensorsID, ISensor*> sensorsMap;
		
		// мапа таймеров
		std::map<SensorsID, TimerHandle_t> timersMap;
		
		// мапа адресов натроек id - addr
		std::map<SensorsID, uint32_t> settingAddresMap = 
		{
		{TEMP_SENSOR_1_ID, global_settings_t::getInstance().FREQ_TEMP_SENSOR_1_ADDR},
		{TEMP_SENSOR_2_ID, global_settings_t::getInstance().FREQ_TEMP_SENSOR_2_ADDR},
		{TEMP_SENSOR_3_ID, global_settings_t::getInstance().FREQ_TEMP_SENSOR_3_ADDR},
		{TEMP_SENSOR_C_ID, global_settings_t::getInstance().FREQ_TEMP_SENSOR_C_ADDR},
		{DUST_SENSOR_1_ID, global_settings_t::getInstance().FREQ_DUST_SENSOR_1_ADDR},
		{DUST_SENSOR_2_ID, global_settings_t::getInstance().FREQ_DUST_SENSOR_1_ADDR},
		};
		
		static QueueHandle_t* sensor_event_queue;
		static QueueHandle_t* storage_event_queue;			// writeOnly
		
		bool KILL_PROSESS_FLAG = false;
		
		void fillFunctionMap();
		void saveAllPointer();
		void deleteAllTimers();
		void createSensor(SensorsID id);
		uint32_t getSensorIntervalById(SensorsID);
		static void timers_Callback(TimerHandle_t xTimer);
};

#endif /* MAIN_SENSOR_SENSORMENAGER_SENSORMENAGER_HPP_ */
