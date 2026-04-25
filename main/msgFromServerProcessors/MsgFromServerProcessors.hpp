/*
 * MsgDeserialize.hpp
 *
 *  Created on: 7 апр. 2026 г.
 *      Author: Kirill
 */

#ifndef MAIN_MSGFROMSERVERPROCESSORS_MSGFROMSERVERPROCESSORS_HPP_
#define MAIN_MSGFROMSERVERPROCESSORS_MSGFROMSERVERPROCESSORS_HPP_

#include <map>
#include <functional>
#include "ICollectionProcessor.hpp"
#include "MsgServerTypes.hpp"
#include "collection_t.hpp"
#include "freertos/FreeRTOS.h"
#include "PostSettingCollection.hpp"

class MsgFromServerProcessors final
{
	// Принимает ссылку на запрос
	using collection_func = std::function<void(request_t&)>;
	using event_func      = std::function<void(msg_server_cmd* cmd)>;
	
	public:
		MsgFromServerProcessors();
	  	virtual ~MsgFromServerProcessors();
	  	
	  	void internalProcessor(msg_server_cmd*);
	  	
	    void static overrideInternalQueue(QueueHandle_t* queue);
	    void static overrideStorageQueue(QueueHandle_t* queue);
	  	
	private:
	
		// Вложенный класс обработки сообщений сообщений типа POST
		ICollectionProcessor* post_setting_collectionProcessor;
		
		bool KILL_PROCESS_FLAG = false;
		
		static QueueHandle_t* storage_event_queue;
		static QueueHandle_t* desiral_event_queue;
		
		void fillCollectionFuncMap();
		void fillEventFuncMap();
		
		std::map<collection_t, collection_func> collectionProcessor;
		std::map<server_event_type_t, event_func> eventProcessor;
	
};

#endif /* MAIN_MSGFROMSERVERPROCESSORS_MSGFROMSERVERPROCESSORS_HPP_ */
