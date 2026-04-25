/*
 * Network.cpp
 *
 *  Created on: 19 февр. 2026 г.
 *      Author: Kirill
 */

#include "MsgServerTypes.hpp"
#include "convertFunc.hpp"
#include "NetworkTypes.hpp"
#include "StorageTypes.hpp"
#include "UploaderTypes.hpp"
#include "NetworkMenager.hpp"
#include "global_settings_t.hpp"

QueueHandle_t* NetworkMenager :: upload_event_queue = nullptr;
QueueHandle_t* NetworkMenager :: network_event_queue = nullptr;
QueueHandle_t* NetworkMenager :: storage_event_queue = nullptr;
QueueHandle_t* NetworkMenager :: desirializer_event_queue = nullptr;

NetworkMenager :: NetworkMenager ()
{
	fillFunctionMap();
	
	reloadTimer = xTimerCreate(
		"net timer",
		pdMS_TO_TICKS(5000),
		pdFALSE,
		(void*)START_WIFI_INIT_EVENT,
        networkTimer_restartTask_Callback
    );
}

NetworkMenager::~NetworkMenager ()
{
	delete this -> ws;
	delete this -> mDNS;
	delete this -> wifi;
}


// -------- CallBack функции для классов соединения

void NetworkMenager :: wifiHundlerStatic(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
	 NetworkMenager* self = static_cast<NetworkMenager*>(arg);
     self -> wifiHundlerReal(arg, event_base, event_id, event_data);
}

void NetworkMenager :: wifiHundlerReal(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{			
	if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
		network_cmd_t* cmd = new network_cmd_t;
			
		printf("wifi is init start connect\n");
		
		cmd -> event_type = START_WIFI_CONNECT_EVENT;
		
		xQueueSend(*network_event_queue, &cmd, 0);
        return;
    }
    
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {		
		printf("WIFI disconect\n");
		
		this -> try_reconect_count++;
		network_cmd_t* cmd = new network_cmd_t;	
		try_reconect_count > 5 ? 
			cmd -> event_type = WIFI_RESET_SETTING :
			cmd -> event_type = START_WIFI_DIS_CONNECT_EVENT;
		
		xQueueSend(*network_event_queue, &cmd, 0);
        return;
    }
    
    if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
		network_cmd_t* cmd = new network_cmd_t;
		ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        
		printf("WiFi Got IP address: %d.%d.%d.%d\n", IP2STR(&event -> ip_info.ip));
		
		cmd -> event_type = START_MDNS_INIT_EVENT;
		xQueueSend(*network_event_queue, &cmd, 0);
	}
}

void NetworkMenager :: wsHundlerStatic(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
	NetworkMenager* self = static_cast<NetworkMenager*>(arg);
	self -> wsHundlerReal(arg, event_base, event_id, event_data);
}

void NetworkMenager :: wsHundlerReal(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *) event_data;
	
    switch (event_id)
    {
		// Если произошел коннект
        case WEBSOCKET_EVENT_CONNECTED:
		{
			printf("WEBSOCKET connect!\n");
			
			storage_cmd_t* stor_cmd = new storage_cmd_t;
			stor_cmd -> event_type = UPDATE_FROM_SERVER;
			stor_cmd -> sync_semaphore = NULL;
			
			xQueueSend(*storage_event_queue, &stor_cmd , 0);
			
			network_cmd_t* net_cmd = new network_cmd_t;
			net_cmd -> event_type = START_SEND_MESSAGE;
			
			xQueueSend(*network_event_queue, &net_cmd, 0);
			
            break;
		}

        case WEBSOCKET_EVENT_DISCONNECTED:
		{			
			printf("WEBSOCKET disconnect\n");
			
			network_cmd_t* net_cmd_1 = new network_cmd_t;
			net_cmd_1 -> event_type = STOP_SEND_MESSAGE;
			
			network_cmd_t* net_cmd_2 = new network_cmd_t;
			net_cmd_2 -> event_type = START_WEBSOCKET_DIS_CONNECT_EVENT;

			xQueueSend(*network_event_queue, &net_cmd_1, 0);
			xQueueSend(*network_event_queue, &net_cmd_2, 0);
						
            break;
		}

		// Если пришли данные
        case WEBSOCKET_EVENT_DATA:
        {
			printf("I get DATA with lenght %d\n", data -> data_len);
            if(data -> data_len <= 1)
            {
				return;
			}
			
            // Выводим полученные данные
            // printf("Data: %.*s\n", data -> data_len, (network_cmd_t*)data -> data_ptr);
            msg_server_cmd* cmd_processor = new msg_server_cmd;
            cmd_processor -> event_type = PROCESS;
   			cmd_processor -> inputString = new char[data -> data_len + 1];
            cmd_processor -> inputString[data -> data_len] = '\0';
            
            memcpy(cmd_processor -> inputString, (char*)data -> data_ptr, data -> data_len);
            xQueueSend(*desirializer_event_queue, &cmd_processor, 0);
            
            break;
		}

        case WEBSOCKET_EVENT_ERROR:
        {
			printf("Error WebSocket\n");
            break;
		}
    }
}

// ------------- Функции обработки событий

void NetworkMenager :: setWifiSetting(wifi_settings_t* wifi_settings)
{
	this -> wifi_settings = wifi_settings;
}

void NetworkMenager :: overrideInternalQueue(QueueHandle_t* queue)
{
	network_event_queue = queue;
}

void NetworkMenager :: overrideStorageQueue(QueueHandle_t *queue)
{
	storage_event_queue = queue;
}

void NetworkMenager :: overrideUploadQueue(QueueHandle_t *queue)
{
	upload_event_queue = queue;
}

void NetworkMenager :: overrideDesirializerQueue(QueueHandle_t *queue)
{
	desirializer_event_queue = queue;
}

void NetworkMenager :: eventProcessor(network_cmd_t* cmd)
{
	if(!cmd)
	{
		printf("Error cmd is NULL (NetworkMenager.eventProcessor)\n");
		return;
	}
	
	if(cmd -> event_type < 0 || cmd -> event_type > 14)
	{
		printf("Error cmd event or cmd data incorrect (NetworkMenager.eventProcessor)\n");
		return;
	}
	
	if(!functionMap.contains(cmd -> event_type)) {
		printf("Error cmd event dose not exists (NetworkMenager.eventProcessor)\n");
		return;	
	}
	
	esp_err_t err = functionMap[cmd -> event_type](cmd);
	
	
	if (err != ESP_OK)
	{
		network_cmd_t* cmd_pointer = new network_cmd_t(*cmd);
		vTimerSetTimerID(reloadTimer, (void*)cmd_pointer); // значение указателя
    	xTimerStart(reloadTimer, pdMS_TO_TICKS(100));
	}
	
	if(cmd -> sync_semaphore == NULL) {
		delete cmd;
		return;
	}
		
	xSemaphoreGive(cmd -> sync_semaphore);
}

void  NetworkMenager :: fillFunctionMap()
{
	functionMap[INIT_SETTINGS]                     = [this] (network_cmd_t*){
			
		if(!storage_event_queue) return ESP_ERR_INVALID_CRC;
		
		this -> wifi_settings = new wifi_settings_t();
		
		global_settings_t* settings = &global_settings_t::getInstance();
		storage_cmd_t* storage_cmd = new storage_cmd_t;
		
		storage_cmd -> event_type = READ_DATA;
		storage_cmd -> sync_semaphore = xSemaphoreCreateBinary();
		storage_cmd -> data_size = 2;
		storage_cmd -> sectorAddr = settings -> WIFI_PASS_ADDR;
		
		storage_cmd -> data[0].length = 32;
		storage_cmd -> data[0].addr = settings -> WIFI_NAME_ADDR;
		
		storage_cmd -> data[1].length = 64;
		storage_cmd -> data[1].addr = settings -> WIFI_PASS_ADDR;
		
		xQueueSend(*storage_event_queue, &storage_cmd, portMAX_DELAY);
		xSemaphoreTake(storage_cmd -> sync_semaphore, portMAX_DELAY);
		vSemaphoreDelete(storage_cmd -> sync_semaphore);
		
		storage_cmd -> data[0].data[31] = '\0';	// страховка
		storage_cmd -> data[1].data[63] = '\0'; // страховка
		
		printf("wi-fi name - %s\n", storage_cmd -> data[0].data);
		printf("wi-fi pass - %s\n", storage_cmd -> data[1].data);
				
		if(arrayContainsTrush(storage_cmd -> data[0].data, 32)) {
			
			settings -> getDefaultSettingByAddr(settings -> WIFI_NAME_ADDR, this -> wifi_settings -> wifi_name);
		}
		else {
			memcpy(this -> wifi_settings -> wifi_name, storage_cmd -> data[0].data, 32);
		}
			
		if(arrayContainsTrush(storage_cmd -> data[1].data, 64)) {
			
			settings -> getDefaultSettingByAddr(settings -> WIFI_PASS_ADDR, this -> wifi_settings -> wifi_pass);
		}
		else {
			memcpy(this -> wifi_settings -> wifi_pass, storage_cmd -> data[1].data, 64);
		}			
				
		network_cmd_t* cmd = new network_cmd_t;
		cmd -> event_type = START_WIFI_INIT_EVENT;

		xQueueSend(*network_event_queue, &cmd, 0);
		delete storage_cmd;
		
		return ESP_OK;
	};
	
	functionMap[START_WIFI_INIT_EVENT] 			   = [this] (network_cmd_t*){

		if(!this -> wifi_settings)
		{
			return ESP_ERR_WIFI_CONN;
		}

		this -> wifi = new WIFI(this -> wifi_settings);
		this -> wifi -> overrideHundler(this -> wifiHundlerStatic, this);
		return this -> wifi -> wifiInit();
	};
	
	functionMap[START_WIFI_CONNECT_EVENT]		   = [this] (network_cmd_t*) { 
		return this -> wifi -> wifiConnect();
	};
	
	functionMap[WIFI_RESET_SETTING]                =  [this] (network_cmd_t*) {
		global_settings_t* settings = &global_settings_t::getInstance();
		settings -> getDefaultSettingByAddr(settings -> WIFI_NAME_ADDR, this -> wifi_settings -> wifi_name);
		settings -> getDefaultSettingByAddr(settings -> WIFI_PASS_ADDR, this -> wifi_settings -> wifi_pass);
		
		//delete this -> wifi;
		// this -> wifi -> setWifiConfig(this -> wifi_settings);
		// this -> wifi -> wifiInit();
		this -> wifi -> updateAndReconnect(this -> wifi_settings);
				
		return ESP_OK;
	};
	
	functionMap[START_WIFI_DIS_CONNECT_EVENT]      = [this] (network_cmd_t*) {
		// Чтобы избежать генерации лишних событий
		
		if (this -> ws) {
        	delete this -> ws;
        	this -> ws = nullptr;
    	}
    	
    	if(this -> mDNS) {
			delete this -> mDNS;
			this -> mDNS =  nullptr;	
		}
    	
    	network_cmd_t* cmd = new network_cmd_t;
    	cmd -> event_type = START_WIFI_CONNECT_EVENT;
    	
    	xQueueSend(*network_event_queue, &cmd, 0);
		return ESP_OK;
	};

    functionMap[START_MDNS_INIT_EVENT]      	   = [this] (network_cmd_t*) {
		this -> wifi ->setConnectedFlag();
		this -> mDNS = new MulticastDNS();
		
		if(this -> mDNS -> initMDNS() != ESP_OK)
		{
			return ESP_ERR_WIFI_NOT_INIT;
		}
		
		network_cmd_t* cmd = new network_cmd_t;
    	cmd -> event_type = START_MDNS_FIND_URL_EVENT;
    	
		xQueueSend(*network_event_queue, &cmd, 0);
		return ESP_OK;
	};
	
	functionMap[START_MDNS_FIND_URL_EVENT]  	   = [this] (network_cmd_t*) {
		if(this -> mDNS -> findServerURL() != ESP_OK)
		{
			return ESP_ERR_WIFI_NOT_INIT;
		}
		
		strlcpy(this -> serverURL, this -> mDNS -> getServerUrl(), 64);
		
		network_cmd_t* cmd = new network_cmd_t;
    	cmd -> event_type = START_WEBSOCKET_INIT_EVENT;
    	
		xQueueSend(*network_event_queue, &cmd, 0);
		return ESP_OK;
	};
	
	functionMap[START_MDNS_DIS_CONECT_EVENT]   	   = [this] (network_cmd_t*) {
		delete this -> mDNS;
		vTaskDelay(pdMS_TO_TICKS(5000));
		
		network_cmd_t* cmd = new network_cmd_t;
    	cmd -> event_type = START_MDNS_INIT_EVENT;
		xQueueSend(*network_event_queue, &cmd, 0);
		
		return ESP_OK;
	};
	
	functionMap[START_WEBSOCKET_INIT_EVENT] 	   = [this] (network_cmd_t*) {	
		this -> ws = new WebSocket();
		this -> ws -> ovverideHundler(this -> wsHundlerStatic, this);
		
		if(this -> ws -> webSocketInit(this -> serverURL) != ESP_OK)
		{
			return ESP_ERR_WIFI_NOT_INIT;
		}
		
		network_cmd_t* cmd = new network_cmd_t;
    	cmd -> event_type = START_WEBSOCKET_CONNECT_EVENT;
		xQueueSend(*network_event_queue, &cmd, 0);
		return ESP_OK;
	};

	functionMap[START_WEBSOCKET_CONNECT_EVENT] 	   = [this] (network_cmd_t*){
		return ws -> webSocketConnect();
	};

	functionMap[START_WEBSOCKET_DIS_CONNECT_EVENT] = [this] (network_cmd_t*)	{
		if (this -> ws) {
        	delete this -> ws;
        	this -> ws = nullptr;
    	}
		
		network_cmd_t* cmd = new network_cmd_t;
    	cmd -> event_type = START_MDNS_DIS_CONECT_EVENT;
		xQueueSend(*network_event_queue, &cmd, 0);
	    return ESP_OK;
	};

	functionMap[SEND_MESSAGE] 			           = [this] (network_cmd_t* cmd ) {
			
		this -> sendRequestToServer(cmd -> data);
		return ESP_OK;
	};
	
	functionMap[START_SEND_MESSAGE] 			   = [] (network_cmd_t*){
			
		uploader_cmd_t* upload_cmd = new uploader_cmd_t;
		upload_cmd -> event_type = START_GLOBAL_UPLOAD;
		upload_cmd -> sync_semaphore = NULL;
		
		xQueueSend(*upload_event_queue, &upload_cmd, 0);
		return ESP_OK;
	};
	
	functionMap[STOP_SEND_MESSAGE] 			       = [] (network_cmd_t*) {
		uploader_cmd_t* upload_cmd = new uploader_cmd_t;
		upload_cmd -> event_type = STOP_GLOBAL_UPLOAD;
		upload_cmd -> sync_semaphore = NULL;
		
		xQueueSend(*upload_event_queue, &upload_cmd, 0);
		return ESP_OK;
	};

	functionMap[SHUTDOWN_NETWORK] 			       = [this] (network_cmd_t* cmd) {
		
		if (this -> ws) {
        	delete this -> ws;
        	this -> ws = nullptr;
    	}
    	
    	if(this -> mDNS) {
			delete this -> mDNS;
			this -> mDNS =  nullptr;	
		}
		
		if(this -> wifi) {
			delete this -> wifi;
			this -> wifi = nullptr;
		}
		
		return ESP_OK;
	};
}

esp_err_t NetworkMenager :: sendRequestToServer(char* message)
{
	if(!this -> wifi) return ESP_ERR_WIFI_DISCARD;
	if(!this -> mDNS) return ESP_ERR_WIFI_DISCARD;
	if(!this -> ws)   return ESP_ERR_WIFI_DISCARD;
	if(!this -> ws -> websocket_client_is_connected()) return ESP_ERR_WIFI_DISCARD;
	
	this -> ws -> esp_websocket_send(message);
	
	return ESP_OK;
}

void NetworkMenager :: networkTimer_restartTask_Callback(TimerHandle_t xTimer)
{
	network_cmd_t* cmd = static_cast<network_cmd_t*>(pvTimerGetTimerID(xTimer));
    
    xQueueSend(*network_event_queue, &cmd, 0);
}

