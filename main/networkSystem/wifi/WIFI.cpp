/*
 * WIFI.cpp
 *
 *  Created on: 15 февр. 2026 г.
 *      Author: Kirill
 */

#include "WIFI.hpp"
#include "esp_wifi.h"

WIFI::WIFI (wifi_settings_t* wifi_settings)
{
	this -> wifi_settings = wifi_settings;
}

WIFI::~WIFI ()
{
	if(WIFI_CONNECTED_FLAG) {
		printf("start wifi disconnect\n");
		esp_wifi_disconnect();
		WIFI_CONNECTED_FLAG = false;
		
		 vTaskDelay(pdMS_TO_TICKS(100));
	}
	
	if(WIFI_STARTED_FLAG) {
		printf("start wifi stop\n");
		esp_wifi_stop();
		WIFI_STARTED_FLAG = false;
		
		 vTaskDelay(pdMS_TO_TICKS(100));
	}
	
	if(sta_netif != nullptr) {
		esp_netif_destroy(sta_netif);
		sta_netif = nullptr;
        printf("WiFi station netif destroyed\n");
	}
	
	if(WIFI_INITIALIZED_FLAG) {
		esp_wifi_deinit();
		WIFI_INITIALIZED_FLAG = false;
        printf("WiFi deinitialized\n");
	}
	
	if(WIFI_NETIF_INIT) {
		esp_netif_deinit();
		printf("start netif deinit\n");
		WIFI_NETIF_INIT = false;
	}
}

esp_err_t WIFI :: initNetif()
{
	esp_err_t err;
	
	 // Инициализация сетевого стека
    if((err = esp_netif_init()) != ESP_OK)
	{
		// printf("error init TCP/IP - WIFI.wifiConnected\n");
		return err;
	}
        
    // Создаем событийную петлю
    if((err = esp_event_loop_create_default()) != ESP_OK)
    {
		// printf("error crreate event loop - WIFI.wifiConnected\n");
		return err;
	}
	
	WIFI_NETIF_INIT = true;
	
	return ESP_OK;
}

esp_err_t WIFI :: wifiInit()
{	
	esp_err_t err = ESP_OK;
	
	// Сетевой стек
	if(!WIFI_NETIF_INIT) {
		err = initNetif();
	}
	
	if(err != ESP_OK) {
		printf("error init netif - (WIFI.wifiInit) %d\n", err);
		return err;
	}
    
    // Создаем станционный интерфейс
    if(this -> sta_netif == nullptr) {
		this -> sta_netif = esp_netif_create_default_wifi_sta();
	}

    // Базовая конфигурация Wi-Fi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    
    if((err = esp_wifi_init(&cfg)) != ESP_OK) {
		printf("error init wifi - (WIFI.wifiInit) %d\n", err);
		return err;
	}
	
	// Регистрирцем обработчики
	wifiRegHundlers();
	
    // Устанавливаем режим станции
    if((err = esp_wifi_set_mode(WIFI_MODE_STA)) != ESP_OK)
    {
		printf("error init station mode - (WIFI.wifiInit) \n");
		return err;
	}
   
    // Настройка конфигурации Wi-Fi    
    wifi_config_t wifi_config = {};
    getWifiConfig(wifi_config);
    
    // Применяем конфигурацию
    if((err = esp_wifi_set_config(WIFI_IF_STA, &wifi_config))!= ESP_OK)
    {
		printf("error set wifi config - (WIFI.wifiInit) \n");
		return err;
	}
	
	 // Запускаем Wi-Fi
	if((err = esp_wifi_start()) != ESP_OK)
	{
		printf("error start wifi - (WIFI.wifiInit) %d\n", err);
		return err;
	}
	
	WIFI_STARTED_FLAG = true;
	
    return ESP_OK;
}


void WIFI::getWifiConfig(wifi_config_t &wifi_config)
{
	memset(&wifi_config, 0, sizeof(wifi_config));
	
	// Копируем NAME (гарантируем нулевой терминатор)
	size_t name_len = strnlen((const char*)wifi_settings -> wifi_name, 32);
	memcpy(wifi_config.sta.ssid, this -> wifi_settings -> wifi_name, name_len);
	
	// wifi_config.sta.ssid[sizeof(wifi_config.sta.ssid) - 1] = '\0';
	// printf("got wifi name: %s\n", this -> wifi_settings -> wifi_name);

	// Копируем пароль
	size_t pass_len = strnlen((const char*)wifi_settings -> wifi_pass, 64);
	memcpy(wifi_config.sta.password, this -> wifi_settings -> wifi_pass, pass_len);
	
	// wifi_config.sta.password[sizeof(wifi_config.sta.password) - 1] = '\0';
	// printf("got wifi pass: %s\n", this -> wifi_settings -> wifi_pass);
}

esp_err_t WIFI :: wifiConnect()
{	
	return esp_wifi_connect();
}


void WIFI :: wifiRegHundlers()
{
	if(this -> wifiHundler == NULL || this -> handlerContext == NULL)
	{
		// printf("is absent hundler wifi function\n");
		return;
	}
	
	esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID,
    	this -> wifiHundler,
		this -> handlerContext, NULL);
		
	esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP,
    	this -> wifiHundler,
		this -> handlerContext, NULL);
}

void WIFI :: setConnectedFlag()
{
	this -> WIFI_CONNECTED_FLAG = true;
}

void WIFI :: overrideHundler(WifiHundler hundler, void* context)
{
	this -> wifiHundler = hundler;
	this -> handlerContext = context;
}

esp_err_t WIFI::updateAndReconnect(wifi_settings_t* new_settings)
{
    if (!WIFI_STARTED_FLAG)
    {
        // Если WiFi не запущен, просто сохраняем настройки и запускаем обычную инициализацию
        this -> wifi_settings = new_settings;
        return wifiInit();
    }

    // Отключаемся (если были подключены)
    if (WIFI_CONNECTED_FLAG)
    {
        esp_wifi_disconnect();
        WIFI_CONNECTED_FLAG = false;
        // Даём время на обработку отключения
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    // Обновляем настройки
    this -> wifi_settings = new_settings;

    // Применяем конфигурацию
    wifi_config_t wifi_config = {};
    getWifiConfig(wifi_config);

    esp_err_t err = esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    if (err != ESP_OK) {
        printf("Set config error: %s\n", esp_err_to_name(err));
        return err;
    }

    // Запускаем подключение к новой сети
    err = esp_wifi_connect();
    if (err != ESP_OK) {
        printf("Connect error: %s\n", esp_err_to_name(err));
        return err;
    }

    printf("Reconnecting with new settings...\n");
    return ESP_OK;
}
