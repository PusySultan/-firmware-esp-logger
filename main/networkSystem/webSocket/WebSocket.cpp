/*
 * WebSocket.cpp
 *
 *  Created on: 19 февр. 2026 г.
 *      Author: Kirill
 */

#include "WebSocket.hpp"

WebSocket :: WebSocket ()
{
}

WebSocket :: ~WebSocket ()
{
	if (client == nullptr)
	{
		return;
    }
    
    esp_websocket_client_destroy(client);
    client = nullptr;
}

esp_err_t WebSocket :: webSocketInit(char* serverUrl)
{	
	// Конфигурация WebSocket
    esp_websocket_client_config_t websocket_cfg = {
        .uri = serverUrl,						// Полный URI сервера WebSocket
        .port = 8080,							// Номер порта сервера
        .disable_auto_reconnect = true,			// Отключает автоматическое переподключение
        .task_stack = 8192,						// Размер стека для внутренней клиентской задачи WebSocket
		.cert_pem = NULL,						// Указатель на строку PEM с сертификатом ЦС сервера для подключений по протоколу WSS
		.skip_cert_common_name_check = true, 	// Не проверять имя сертификата
		.reconnect_timeout_ms = 5000,			
		.network_timeout_ms = 5000
    };
    
    client = esp_websocket_client_init(&websocket_cfg);
    
    if (client == NULL)
    {
        // printf("Init error WebSocket client\n");
        return ESP_ERR_INVALID_STATE;
    }

    // Регистрируем обработчик событий WebSocket
    if(esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY,
    wsHundler, this -> handlerContext) != ESP_OK)
    {
		return ESP_ERR_INVALID_ARG;
	}

	return ESP_OK;
}

esp_err_t WebSocket :: webSocketConnect()
{
    if(esp_websocket_client_start(client) != ESP_OK)
    return ESP_ERR_INVALID_RESPONSE;
	return ESP_OK;
}

void WebSocket :: esp_websocket_send(char message[])
{
	esp_websocket_client_send_text(client, message, strlen(message), portMAX_DELAY);
}

bool WebSocket :: websocket_client_is_connected()
{
	if (!client) return false;
	
	return esp_websocket_client_is_connected(client);
}

void WebSocket :: ovverideHundler(WebSocketHundler hundler, void* context)
{
	this -> wsHundler = hundler;
	this -> handlerContext = context;
}






