/*
 * NetworkTypes.hpp
 *
 *  Created on: 24 февр. 2026 г.
 *      Author: Kirill
 */

#ifndef MAIN_NETWORKSYSTEM_NETWORKTYPES_HPP_
#define MAIN_NETWORKSYSTEM_NETWORKTYPES_HPP_

#include <cstdint>
#include "freertos/FreeRTOS.h"

enum network_event_type_t
{
	SHUTDOWN_NETWORK = 0,
	
	INIT_SETTINGS = 1,
		
	START_WIFI_INIT_EVENT = 2,
	START_WIFI_CONNECT_EVENT = 3,
	START_WIFI_DIS_CONNECT_EVENT = 4,
	
	START_MDNS_INIT_EVENT = 5,
	START_MDNS_DIS_CONECT_EVENT = 6,
	START_MDNS_FIND_URL_EVENT = 7,
	
	START_WEBSOCKET_INIT_EVENT = 8,
	START_WEBSOCKET_CONNECT_EVENT = 9,
	START_WEBSOCKET_DIS_CONNECT_EVENT = 10,
	
	
	// В будущем нудно переписать эти события, сетевой менеджер
	// должен просто отослать событие подключения, и сами классы на него должны по своему реагировать
	
	START_SEND_MESSAGE = 11,
	STOP_SEND_MESSAGE  = 12,
	      SEND_MESSAGE = 13,
	      WIFI_RESET_SETTING = 14

	
};

struct network_cmd_t
{
	network_event_type_t event_type;
	char data[1024];
	
	SemaphoreHandle_t sync_semaphore = NULL;
};

#endif /* MAIN_NETWORKSYSTEM_NETWORKTYPES_HPP_ */
