/*
 * EventMenagerOverride.cpp
 *
 *  Created on: 30 апр. 2026 г.
 *      Author: Kirill
 */


#include "EventMenager.hpp"

QueueHandle_t* EventMenager :: event_queue;
QueueHandle_t* EventMenager :: notif_event_queue;
QueueHandle_t* EventMenager :: create_event_queue;
QueueHandle_t* EventMenager :: sensor_event_queue;
QueueHandle_t* EventMenager :: upload_event_queue;
QueueHandle_t* EventMenager :: storage_event_queue;
QueueHandle_t* EventMenager :: network_event_queue;
QueueHandle_t* EventMenager :: serverMsgProcessor_event_queue;


void EventMenager :: overrideInternalQueue(QueueHandle_t* queue)
{
	event_queue = queue;
}

void EventMenager :: overrideStorageQueue(QueueHandle_t *queue)
{
	storage_event_queue = queue;
}

void EventMenager :: overrideSensorQueue(QueueHandle_t *queue)
{
	sensor_event_queue = queue;
}

void EventMenager :: overrideUploadQueue(QueueHandle_t *queue)
{
	upload_event_queue = queue;
}

void EventMenager :: overrideNetworkQueue(QueueHandle_t *queue)
{
	network_event_queue = queue;
}

void EventMenager :: overrideCreateQueue(QueueHandle_t *queue)
{
	create_event_queue = queue;
}

void EventMenager :: overrideSerMsgProcessQueue(QueueHandle_t* queue)
{
	serverMsgProcessor_event_queue = queue;
}

void EventMenager :: overrideNotifQueue(QueueHandle_t* queue)
{
	notif_event_queue = queue;
}