/*
 * Uploader.hpp
 *
 *  Created on: 19 мар. 2026 г.
 *      Author: Kirill
 */

#ifndef MAIN_UPLOADER_UPLOADER_HPP_
#define MAIN_UPLOADER_UPLOADER_HPP_

#include <map>
#include <functional>
#include "collection_t.hpp"
#include "UploaderTypes.hpp"
#include "freertos/FreeRTOS.h"

class Uploader final
{
	using func = std::function<void(uploader_cmd_t* cmd)>;
	
	public:
	  Uploader ();
	  virtual ~Uploader ();
	  
	  void fillFunctionMap();
	  void eventProcessor(uploader_cmd_t* cmd);
	  
	  void static overrideInternalQueue(QueueHandle_t* queue);
	  void static overrideStorageQueue(QueueHandle_t* queue);
	  void static overrideCreaterQueue(QueueHandle_t* queue);
	  
	private:
	    bool KILL_PROSESS_FLAG = false;
	    bool ADDR_FINDED = false;
	    bool OLD_DATA_UPLOAD = false;
	    bool STOP_UPOLAD = false;
	    
		uint8_t uploadIteration = 0;
		
		static QueueHandle_t* upload_event_queue;
		static QueueHandle_t* storage_event_queue;
		static QueueHandle_t* creater_event_queue;
	
		// Мапа функций обработки 
		std::map<uint8_t, func> functionMap;
		
		// Мапа адресов загрузки (датчики и состояния)
		std::map<SensorsID, uint32_t> uploadAddr
		{
			{TEMP_SENSOR_1_ID,		UINT32_MAX},
			{TEMP_SENSOR_2_ID,		UINT32_MAX},
			{TEMP_SENSOR_3_ID,		UINT32_MAX},
			{TEMP_SENSOR_C_ID,		UINT32_MAX},
			{DUST_SENSOR_1_ID,		UINT32_MAX},
			{DUST_SENSOR_2_ID,		UINT32_MAX},
			{CASE_OPEN_EVENT_ID,	UINT32_MAX},
			{CASE_CLOSE_EVENT_ID, 	UINT32_MAX}

		};
		
		// Мапа состояний
		std::map<SensorsID, bool> uploadStateById
		{	// true - все данные загружены для данного датчика
			{TEMP_SENSOR_1_ID,  	false},
			{TEMP_SENSOR_2_ID,  	false},
			{TEMP_SENSOR_3_ID,  	false},
			{TEMP_SENSOR_C_ID, 		false},
			{DUST_SENSOR_1_ID, 		false},
			{DUST_SENSOR_2_ID,  	false},
			{CASE_OPEN_EVENT_ID, 	false},
			{CASE_CLOSE_EVENT_ID, 	false},
		};
		
		bool checkAllStates();
		void reesteAllStates();
		void getNextAddr(SensorsID id);
		void resetFlagById(SensorsID id);
		void uploadAddrById(SensorsID id);
		void readFlag(uint32_t adddr, uint8_t* flag);
		void sendToServer(SensorsID id, uint8_t* data, uint8_t* dt);
		void uploadDataByAddr(uint32_t addr, uint8_t* data, uint8_t* dt);

		collection_t getCmdCollection(SensorsID id);
		SensorsID getNextId(SensorsID id);
};

#endif /* MAIN_UPLOADER_UPLOADER_HPP_ */
