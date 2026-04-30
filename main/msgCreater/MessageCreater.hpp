/*
 * MessageCreater.hpp
 *
 *  Created on: 19 мар. 2026 г.
 *      Author: Kirill
 */

#ifndef MAIN_MSGCREATER_MESSAGECREATER_HPP_
#define MAIN_MSGCREATER_MESSAGECREATER_HPP_

#include <map>
#include "cJSON.h"
#include <functional>
#include "freertos/FreeRTOS.h"
#include "MessageCreaterTypes.hpp"

class MessageCreater final
{
	using func = std::function<void(create_cmd_t*)>;
	
	public:
	  MessageCreater ();
	  virtual ~MessageCreater ();
	  
	  void fillReqInfo();
	  void eventProcessor(create_cmd_t* cmd);
	  
	  static void overrideInternalQueue(QueueHandle_t* queue);
	  static void overrideNetworkQueue(QueueHandle_t* queue);
	  static void overridStorageQueue(QueueHandle_t* queue);
	  
	private:
	  bool KILL_PROSESS_FLAG = false;
	
	  cJSON* requesterInfo;
	  std::map<collection_t, func> functionMap;
	  static QueueHandle_t* create_event_queue;
	  static QueueHandle_t* network_event_queue;
	  static QueueHandle_t* storage_event_queue;
	  
	  void killProcess();
	  void fillFunctionMap();
	  void sendToServer(char*);

	  uint64_t getTotalJobInterval();
	  uint64_t getLastJobInterval();
};

#endif /* MAIN_MSGCREATER_MESSAGECREATER_HPP_ */
