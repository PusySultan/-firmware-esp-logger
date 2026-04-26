
#include "main.hpp"
#include "EventMenager.hpp"
#include "memory_w25q_const.hpp"
#include "nvs_flash.h"
#include "EventTypes.hpp"
#include "DateTimeSensor.hpp"
#include "pins_config_const.hpp"
#include "main_tasks.cpp"
#include <stdlib.h>

extern "C" void app_main(void)
{
	wakeup =  esp_sleep_get_wakeup_cause();
	
	if(wakeup == ESP_SLEEP_WAKEUP_EXT0) {
		startSpecialInit();
		goto end;	
	}
	
	startBaseInit();
	createTasks();

	while(true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	}

	end:;
}

void startBaseInit()
{
	nvs_flash_init();
	DateTimeSensor :: getInstance().ds1302_init(CLOCK_ENA_PIN, CLOCK_CLK_PIN, CLOCK_DAT_PIN); // 26, 14, 27
	
	storage = new Storage();
	network = new NetworkMenager();
	sensor = new SensorMenager();
	uploader = new Uploader();
	creater = new MessageCreater();
	server_msg_processor = new MsgFromServerProcessors();
	eventMenager = new EventMenager();
	
	network_event_queue = xQueueCreate(3, sizeof(network_cmd_t*));
	storage_event_queue = xQueueCreate(10,  sizeof(storage_cmd_t*));
	sensor_event_queue  = xQueueCreate(20,  sizeof(sensor_cmd_t*));
	upload_event_queue  = xQueueCreate(10,  sizeof(uploader_cmd_t*));
	creater_event_queue  = xQueueCreate(10, sizeof(create_cmd_t*));
	serverMsgProcessor_event_queue = xQueueCreate(10, sizeof(char*));
	event_queue = xQueueCreate(5, sizeof(event_cmd_t*));
	
	network -> overrideInternalQueue(&network_event_queue);
	network -> overrideStorageQueue(&storage_event_queue);
	network -> overrideUploadQueue(&upload_event_queue);
	network -> overrideDesirializerQueue(&serverMsgProcessor_event_queue);
	
	storage -> overrideInternalQueue(&storage_event_queue);
	storage -> overrideUploadQueue(&upload_event_queue);
	storage -> overrideMsgCreaterQueue(&creater_event_queue);
	
	sensor -> overrideInternalQueue(&sensor_event_queue);
	sensor -> overrideStorageQueue(&storage_event_queue);
	
	uploader -> overrideInternalQueue(&upload_event_queue);
	uploader -> overrideStorageQueue(&storage_event_queue);
	uploader -> overrideCreaterQueue(&creater_event_queue);
	
	creater -> overrideInternalQueue(&creater_event_queue);
	creater -> overrideNetworkQueue(&network_event_queue);
	
	server_msg_processor -> overrideInternalQueue(&serverMsgProcessor_event_queue);
	server_msg_processor -> overrideStorageQueue(&storage_event_queue);
	
	eventMenager -> overrideInternalQueue(&event_queue);
	eventMenager -> overrideStorageQueue(&storage_event_queue);
	eventMenager -> overrideNetworkQueue(&network_event_queue);
	eventMenager -> overrideSensorQueue(&sensor_event_queue);
	eventMenager -> overrideUploadQueue(&upload_event_queue);
	eventMenager -> overrideCreateQueue(&creater_event_queue);
	eventMenager -> overrideSerMsgProcessQueue(&serverMsgProcessor_event_queue);
	
	sendStartEvents();
}

void sendStartEvents()
{
	DateTime dt;
	DateTimeSensor::getInstance().ds1302_getDateTime(&dt);
	
	event_cmd_t* cmd = new event_cmd_t;
	cmd -> event_type = DEVICE_EVENT_ON;
	cmd -> dateTime = dt;
	
	xQueueSend(event_queue, &cmd, 0);
}

void clearQueue()
{
	vQueueDelete(network_event_queue);
	vQueueDelete(storage_event_queue);
	vQueueDelete(sensor_event_queue);
	vQueueDelete(upload_event_queue);
	vQueueDelete(creater_event_queue);
	vQueueDelete(serverMsgProcessor_event_queue);
	vQueueDelete(event_queue);
}

void byby()
{
	esp_sleep_enable_ext0_wakeup((gpio_num_t)WAKEUP_PIN, 1);
	esp_deep_sleep_start();
}

void startSpecialInit()
{
	storage = new Storage();
	storage_event_queue = xQueueCreate(1,  sizeof(storage_cmd_t*));
	storage -> overrideInternalQueue(&storage_event_queue);

	DateTimeSensor :: getInstance().ds1302_init(CLOCK_ENA_PIN, CLOCK_CLK_PIN, CLOCK_DAT_PIN); // 26, 14, 27
}

void regCaseOpening()
{
	DateTime dateTime;
	DateTimeSensor :: getInstance().ds1302_getDateTime(&dateTime);

	storage_cmd_t* storage_cmd = new storage_cmd_t;
		
	storage_cmd -> event_type = WRITE_BY_TRANS;
	storage_cmd -> sync_semaphore = NULL;
	storage_cmd -> data_size  = 1;
	storage_cmd -> sectorAddr = ADDR_EVENT_CASE_OPEN;
	
	memcpy(storage_cmd -> data[0].data, &dateTime, sizeof(dateTime));

	xQueueSend(storage_event_queue, storage_cmd,  pdMS_TO_TICKS(30000));
}

