
#include "main.hpp"
#include "EventMenager.hpp"
#include "nvs_flash.h"
#include "EventTypes.hpp"
#include "DateTimeSensor.hpp"
#include "pins_config_const.hpp"

extern "C" void app_main(void)
{
	initBaseSystem();
	
	xTaskCreate(network_task,
			    "Network task",
			    8192,
			    NULL,
			    8,
			    &networkTaskHundle);
			    
	xTaskCreate(storage_task,
			    "Storage task",
			    4096,
			    NULL,
			    8,
			    &storageTaskHundle);
			    
	xTaskCreate(sensors_task,
			    "Sensors task",
			    4096,
			    NULL,
			    8,
			    &sensorsTaskHundle);
			    
	xTaskCreate(upload_task,
			    "Upload task",
			    4096,
			    NULL,
			    8,
			    &uploadTaskHundle);
			    
	xTaskCreate(create_task,
			    "Create task",
			    4096,
			    NULL,
			    8,
			    &createTaskHundle);
			    
	xTaskCreate(desirializer_task,
			    "Deserialize task",
			    4096,
			    NULL,
			    8,
			    &serverMsgProcessorTaskHundle);
			    
	xTaskCreate(event_task,
			    "Event task",
			    2048,
			    NULL,
			    8,
			    &eventTaskHundle);
			    
	while(true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	}
}

void initBaseSystem()
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

