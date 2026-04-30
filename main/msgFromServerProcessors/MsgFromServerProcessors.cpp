/*
 * MsgDeserialize.cpp
 *
 *  Created on: 7 апр. 2026 г.
 *      Author: Kirill
 */

#include "MsgServerTypes.hpp"
#include "PostSettingCollection.hpp"
#include "collectionFunc.hpp"
#include "request_type_func.hpp"
#include "MsgFromServerProcessors.hpp"

QueueHandle_t* MsgFromServerProcessors :: storage_event_queue;
QueueHandle_t* MsgFromServerProcessors :: desiral_event_queue;

MsgFromServerProcessors :: MsgFromServerProcessors()
{
  this -> post_setting_collectionProcessor = new PostSettingCollection(SETTING);  
  this -> fillCollectionFuncMap();
  this -> fillEventFuncMap();
}

MsgFromServerProcessors :: ~MsgFromServerProcessors ()
{
 	eventProcessor.clear();
}

void MsgFromServerProcessors :: overrideInternalQueue(QueueHandle_t* queue)
{
	desiral_event_queue =  queue;
}

void MsgFromServerProcessors :: overrideStorageQueue(QueueHandle_t *queue)
{
	storage_event_queue = queue;
	PostSettingCollection::overrideStorageQueue(queue);
}

void MsgFromServerProcessors :: internalProcessor(msg_server_cmd* cmd)
{
	if(KILL_PROCESS_FLAG) return;
		
	if(!cmd) {
		printf("ERROR cmd is NULL (internalProcessor.eventProcessor)\n");
	}
	
	if(!eventProcessor.contains(cmd -> event_type)) {
		printf("ERROR dose not exist processor for this type (internalProcessor.eventProcessor)\n");
		return;
	}
	
	eventProcessor[cmd -> event_type](cmd);
}

void MsgFromServerProcessors :: fillCollectionFuncMap()
{
	this -> collectionProcessor[SETTING] = [this](request_t req)
	{
		// req - ссылка на запрос
		
		if(req.request_type == POST)
		{
			this -> post_setting_collectionProcessor -> process(req);
		}
	};
}

void MsgFromServerProcessors :: fillEventFuncMap()
{
	this -> eventProcessor[SHUTDOWN_SERVER_PROCESSOR] = [this] (msg_server_cmd* cmd)
	{
		KILL_PROCESS_FLAG = true;
		collectionProcessor.clear();
		delete this -> post_setting_collectionProcessor;
		
		xQueueReset(*desiral_event_queue);
		
		if(cmd -> sync_semaphore == NULL) {
			delete cmd;
			return;
		}
		
		xSemaphoreGive(cmd -> sync_semaphore);
	};
	
	this -> eventProcessor[PROCESS] = [this] (msg_server_cmd* cmd)
	{
		// ROOT JSON
	  	cJSON* root = cJSON_Parse(cmd -> inputString);
	    if (root == NULL) {
			printf("ERROR root JSON is NULL (MsgFromServerProcessors.eventProcessor)\n");
			cJSON_Delete(root);
	        return;
	    }
	    
	    // COLLECTION
	    collection_t collection = UNKNOW_COLLECTION;
	    cJSON* collection_item = cJSON_GetObjectItem(root, "collection");
		if(collection_item == NULL) {
			return;
		}

	    collection = string_to_collection(collection_item -> valuestring); 
	    
		if(collection == UNKNOW_COLLECTION) {
			printf("ERROR collection is bad: %s (MsgFromServerProcessors.eventProcessor)\n", collection_item -> valuestring);
			cJSON_Delete(root);
			return;
		}
		
		// REQUEST TYPE
		request_type_t request_type;
		cJSON* type_item = cJSON_GetObjectItem(root, "type");
		request_type = string_to_requestType(type_item -> valuestring);
		
		if(request_type == UNKNOW_TYPE) {
			printf("ERROR uknow request type (MsgFromServerProcessors.eventProcessor)\n");
			cJSON_Delete(root);
			return;
		}
	    
	    // MESSAGE
	    cJSON*  message = cJSON_GetObjectItem(root, "message");
	    if (root == NULL) {
			printf("ERROR root JSON is NULL (MsgFromServerProcessors.eventProcessor)\n");
			cJSON_Delete(root);
	        return;
	    }
	    
	    request_t* request = new request_t;
	    request -> request_type = request_type;
	    request -> collection = collection;
	    request -> message = cJSON_Duplicate(message, true);
	        
	    this -> collectionProcessor[collection](*request);
	    
	    cJSON_Delete(request -> message);
	    delete request;
	    
	    cJSON_Delete(root);
	    
	    if(cmd -> sync_semaphore == NULL) {
			delete cmd;
			return;
		}
		
		xSemaphoreGive(cmd -> sync_semaphore);
	};
}
