/*
 * WebSocket.hpp
 *
 *  Created on: 19 февр. 2026 г.
 *      Author: Kirill
 */

#ifndef MAIN_NETWORKSYSTEM_WEBSOCKET_WEBSOCKET_HPP_
#define MAIN_NETWORKSYSTEM_WEBSOCKET_WEBSOCKET_HPP_

#include "esp_websocket_client.h"

class WebSocket
{
	using WebSocketHundler = void (*) (void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
	
	public:
	  WebSocket ();
	  virtual ~WebSocket ();

	  esp_err_t webSocketInit(char* serverUrl);
	  esp_err_t webSocketConnect();
	  bool websocket_client_is_connected();
	  void esp_websocket_send(char message[]);
	  void ovverideHundler(WebSocketHundler hundler, void* context);
	  	  
	private:
		esp_websocket_client_handle_t client;

		WebSocketHundler wsHundler = nullptr;   // Сыылка на функцию обработчик
        void* handlerContext = nullptr;	   		// Ссылка на контекст функции
        
};

#endif /* MAIN_NETWORKSYSTEM_WEBSOCKET_WEBSOCKET_HPP_ */
