/*
 * NetworkSystem.hpp
 *
 *  Created on: 19 февр. 2026 г.
 *      Author: Kirill
 */

#ifndef MAIN_NETWORKSYSTEM_NETWORKMENAGER_HPP_
#define MAIN_NETWORKSYSTEM_NETWORKMENAGER_HPP_

#include "WIFI.hpp"
#include "WebSocket.hpp"
#include "MulticastDNS.hpp"
#include "NetworkTypes.hpp"
#include "NetworkTypes.hpp"

#include <map>
#include <functional>

class NetworkMenager
{
	using func = std::function<esp_err_t(network_cmd_t*)>;
	
	public:
	  NetworkMenager ();
	  virtual ~NetworkMenager ();
	
	  void setWifiSetting(wifi_settings_t* wifi_settings);
	  void eventProcessor(network_cmd_t* cmd);
	  void static overrideInternalQueue(QueueHandle_t* queue);
	  void static overrideStorageQueue(QueueHandle_t* queue);
	  void static overrideUploadQueue(QueueHandle_t* queue);
	  void static overrideDesirializerQueue(QueueHandle_t* queue);
	  
	private:
		WIFI* wifi = nullptr;
		WebSocket* ws= nullptr;
		MulticastDNS* mDNS = nullptr;
		
		uint8_t try_reconect_count = 0;
		wifi_settings_t* wifi_settings = nullptr;
		
		// Мапа функций обработки 
		std::map<uint8_t, func> functionMap;
		char serverURL[64] = {};
		
		// Очередь данных
		static QueueHandle_t* network_event_queue;
		static QueueHandle_t* storage_event_queue;
		static QueueHandle_t* upload_event_queue;
		static QueueHandle_t* desirializer_event_queue;
		
		// Таймер для запуска каллбеков
		TimerHandle_t reloadTimer;
		
		void wifiHundlerReal(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
    	static void IRAM_ATTR wifiHundlerStatic(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

		void wsHundlerReal(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
		static void IRAM_ATTR wsHundlerStatic(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
		
		esp_err_t sendRequestToServer(char*);
					  
		void fillFunctionMap();
		void resetWifiSettingToDefault();
		void static networkTimer_restartTask_Callback(TimerHandle_t xTimer);
};

#endif /* MAIN_NETWORKSYSTEM_NETWORKMENAGER_HPP_ */
