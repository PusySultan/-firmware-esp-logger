/*
 * MessageCreaterTypes.hpp
 *
 *  Created on: 19 мар. 2026 г.
 *      Author: Kirill
 */

#ifndef MAIN_MSGCREATER_MESSAGECREATERTYPES_HPP_
#define MAIN_MSGCREATER_MESSAGECREATERTYPES_HPP_

#include "DateTime.hpp"
#include "SensorTypes.hpp"
#include "collectionFunc.hpp"

enum cmd_type_t
{
	SHUTDOWN_MSG_CREATER = 0,
	CREATE = 1,
};

struct creater_block
{
	uint8_t data[64];				// данные
	uint8_t byte_count;				// колличество байт данных
};

struct create_cmd_t
{
	collection_t collection;
	cmd_type_t cmd_type  = SHUTDOWN_MSG_CREATER;
	
	SensorsID id;					// id устройства для которого выполняется команда
	creater_block createrBlock[16];	// данные
	uint8_t block_length;			// колличество байт данных
	DateTime dateTime;				// Дата и время регистрации
	
	SemaphoreHandle_t sync_semaphore = NULL;
	
};

#endif /* MAIN_MSGCREATER_MESSAGECREATERTYPES_HPP_ */
