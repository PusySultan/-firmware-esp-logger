/*
 * MsgServerTypes.hpp
 *
 *  Created on: 19 апр. 2026 г.
 *      Author: Kirill
 */

#ifndef MAIN_MSGFROMSERVERPROCESSORS_MSGSERVERTYPES_HPP_
#define MAIN_MSGFROMSERVERPROCESSORS_MSGSERVERTYPES_HPP_

#include "freertos/FreeRTOS.h"

enum server_event_type_t
{
	SHUTDOWN_SERVER_PROCESSOR = 0,
	PROCESS = 1,
};

struct msg_server_cmd
{
	server_event_type_t event_type = PROCESS;
	char* inputString;
	SemaphoreHandle_t sync_semaphore = NULL;
};

#endif /* MAIN_MSGFROMSERVERPROCESSORS_MSGSERVERTYPES_HPP_ */
