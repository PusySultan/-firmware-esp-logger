/*
 * Storage.hpp
 *
 *  Created on: 14 мар. 2026 г.
 *      Author: Kirill
 */

#ifndef MAIN_STORAGE_STORAGE_HPP_
#define MAIN_STORAGE_STORAGE_HPP_

#include <map>
#include <functional>
#include "StorageTypes.hpp"
#include "W25Q128.hpp"

class Storage final
{
	using func = std::function<void(storage_cmd_t*)>;
	
	public:
	  Storage (); 
	  virtual ~Storage ();
	  
	   esp_err_t eventProcessor(storage_cmd_t* cmd);
	   void static overrideInternalQueue(QueueHandle_t* queue);
	   void static overrideUploadQueue(QueueHandle_t* queue);
	   void static overrideMsgCreaterQueue(QueueHandle_t* queue);
	  
	private:
	   W25Q128* memory;
	   
	   std::map<storage_event_type_t, func> functionMap;
	   
	   static QueueHandle_t* storage_event_queue;
	   static QueueHandle_t* upload_event_queue;
	   static QueueHandle_t* msg_createre_event_queue;
	   	   
	   void fillFunctionMap();
};

#endif /* MAIN_STORAGE_STORAGE_HPP_ */
