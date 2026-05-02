/*
 * main.hpp
 *
 *  Created on: 14 мар. 2026 г.
 *      Author: Kirill
 */

#ifndef MAIN_MAIN_HPP_
#define MAIN_MAIN_HPP_

#include <esp_sleep.h>
#include "Storage.hpp"
#include "Uploader.hpp"
#include "EventMenager.hpp"
#include "Notification.hpp"
#include "SensorMenager.hpp"
#include "MessageCreater.hpp"
#include "NetworkMenager.hpp"
#include "wekup_sourse_t.hpp"
#include "MsgFromServerProcessors.hpp"

inline esp_sleep_wakeup_cause_t wakeup; 

// class pointers
inline Storage* storage;
inline Uploader* uploader;
inline Notification* notif;
inline SensorMenager* sensor;
inline MessageCreater* creater;
inline NetworkMenager* network;
inline EventMenager* eventMenager;
inline MsgFromServerProcessors* server_msg_processor;

// hundles
inline TaskHandle_t eventTaskHundle = NULL;
inline TaskHandle_t notifTaskHundle = NULL;
inline TaskHandle_t uploadTaskHundle = NULL;
inline TaskHandle_t createTaskHundle = NULL;
inline TaskHandle_t storageTaskHundle = NULL;
inline TaskHandle_t sensorsTaskHundle = NULL;
inline TaskHandle_t networkTaskHundle = NULL;
inline TaskHandle_t serverMsgProcessorTaskHundle = NULL;

// Очереди
inline QueueHandle_t event_queue;
inline QueueHandle_t notif_event_queue;
inline QueueHandle_t upload_event_queue;
inline QueueHandle_t sensor_event_queue;
inline QueueHandle_t create_event_queue;
inline QueueHandle_t network_event_queue;
inline QueueHandle_t storage_event_queue;
inline QueueHandle_t serverMsgProcessor_event_queue;

// Задачи
void event_task(void *pvParameters);
void notif_task(void *pvParameters);
void create_task(void *pvParameters);
void upload_task(void *pvParameters);
void storage_task(void *pvParameters);
void sensors_task(void *pvParameters);
void network_task(void *pvParameters);
void desirializer_task(void *pvParameters);

// Остальные функции
void createTasks();
void startBaseInit();
void overrideQueues();
void sendStartEvents();

	// Функции включения
void regCaseOpening();
void startSpecialInit();

void clearQueue();
void killedTasks();
void byby();    /// уход ко сну
wekup_sourse_t getWekupSourse();


#endif /* MAIN_MAIN_HPP_ */
