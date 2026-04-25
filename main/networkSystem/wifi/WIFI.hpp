/*
 * WIFI.hpp
 *
 *  Created on: 15 февр. 2026 г.
 *      Author: Kirill
 */

#ifndef MAIN_WIFI_WIFI_HPP_
#define MAIN_WIFI_WIFI_HPP_

#include <cstring>
#include "esp_wifi.h"
#include "wifi_settings_t.hpp"

class WIFI
{
	using WifiHundler = void (*)(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
                                  
	public:
	  WIFI (wifi_settings_t* wifi_settings);
	  virtual ~WIFI ();
	  
	  esp_err_t wifiInit();
	  esp_err_t wifiConnect();
	  
	  void setConnectedFlag();
	  
	  void overrideHundler(WifiHundler hundler, void* context);
	  esp_err_t updateAndReconnect(wifi_settings_t* new_settings);
	  
	private:
		// Настройки нашей точки лоступа
		wifi_settings_t* wifi_settings;
		
		esp_err_t initNetif();
		void wifiRegHundlers();
		void getWifiConfig(wifi_config_t &wifi_config);
        
        esp_netif_t* sta_netif = nullptr;	// Указатель на сетевой интерфейс
        WifiHundler wifiHundler = nullptr;  // Сыылка на функцию обработчик
        void* handlerContext = nullptr;	    // Ссылка на контекст функции
        
        bool WIFI_INITIALIZED_FLAG = false;
        bool WIFI_CONNECTED_FLAG = false;
        bool WIFI_STARTED_FLAG = false;
        bool WIFI_NETIF_INIT = false;
        
};

#endif /* MAIN_WIFI_WIFI_HPP_ */
