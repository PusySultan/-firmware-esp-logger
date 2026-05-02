
#include "main.hpp"
#include "Notification.hpp"
#include "NotificationTypes.hpp"
#include "memory_w25q_const.hpp"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "EventMenager.hpp"
#include "DateTimeSensor.hpp"
#include "pins_config_const.hpp"
#include "main_tasks.cpp"

extern "C" void app_main(void)
{
	wekup_sourse_t wsourse = getWekupSourse(); 
	
	if(wsourse == CASE_OPEN_SOURSE)
	{
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
	gpio_install_isr_service(0);
	DateTimeSensor :: getInstance().ds1302_init(CLOCK_ENA_PIN, CLOCK_CLK_PIN, CLOCK_DAT_PIN); // 26, 14, 27
	
	storage = new Storage();
	uploader = new Uploader();
	myNotif = new Notification();
	sensor = new SensorMenager();
	network = new NetworkMenager();
	creater = new MessageCreater();
	eventMenager = new EventMenager();
	server_msg_processor = new MsgFromServerProcessors();
	
	overrideQueues();
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
	vQueueDelete(create_event_queue);
	vQueueDelete(serverMsgProcessor_event_queue);
	vQueueDelete(event_queue);
}

void byby()
{
	const uint64_t WAKEUP_PIN_BITMASK = (1ULL << EVENT_CASE_OPEN_PIN) | (1ULL <<  EVENT_VOLTAGE_OFF_PIN);
	esp_sleep_enable_ext1_wakeup(WAKEUP_PIN_BITMASK, ESP_EXT1_WAKEUP_ANY_HIGH);

	gpio_set_direction(GREEN_COLOR_PIN, GPIO_MODE_OUTPUT);
	gpio_set_level(GREEN_COLOR_PIN, 1);

	esp_deep_sleep_start();
}

void startSpecialInit()
{
	storage = new Storage();
	storage_event_queue = xQueueCreate(1,  sizeof(storage_cmd_t*));
	storage -> overrideInternalQueue(&storage_event_queue);

	DateTimeSensor :: getInstance().ds1302_init(CLOCK_ENA_PIN, CLOCK_CLK_PIN, CLOCK_DAT_PIN); // 26, 14, 27
}

wekup_sourse_t getWekupSourse()
{
	wakeup = esp_sleep_get_wakeup_cause();

	if (wakeup == ESP_SLEEP_WAKEUP_EXT1)
	{
		return DEFAULT_SOURCE;
	}
	
	//Получаем битовую маску конкретных пинов, инициировавших пробуждение
	uint64_t wakeup_pin_mask =  esp_sleep_get_ext1_wakeup_status();

	// Вскрыли корпус
	if (wakeup_pin_mask & (1ULL << EVENT_CASE_OPEN_PIN))
	{
		printf("GPIO CASE OPEN caused the wakeup!\n");
		return CASE_OPEN_SOURSE;
	}
	
	// Дали питание
	if (wakeup_pin_mask & (1ULL << EVENT_VOLTAGE_OFF_PIN))
	{
		printf("GPIO voltage on caused the wakeup!\n");
		return VOLTAGE_ON_SOURSE;
	}

	return DEFAULT_SOURCE;
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

void overrideQueues()
{
	event_queue = xQueueCreate(5, sizeof(event_cmd_t*));
	notif_event_queue = xQueueCreate(3, sizeof(notif_cmd_t*));
	network_event_queue = xQueueCreate(3, sizeof(network_cmd_t*));
	storage_event_queue = xQueueCreate(20,  sizeof(storage_cmd_t*));
	sensor_event_queue  = xQueueCreate(20,  sizeof(sensor_cmd_t*));
	upload_event_queue  = xQueueCreate(10,  sizeof(uploader_cmd_t*));
	create_event_queue  = xQueueCreate(20, sizeof(create_cmd_t*));
	serverMsgProcessor_event_queue = xQueueCreate(10, sizeof(char*));

	storage -> overrideInternalQueue(&storage_event_queue);
	storage -> overrideUploadQueue(&upload_event_queue);
	storage -> overrideMsgCreaterQueue(&create_event_queue);
	
	creater -> overrideInternalQueue(&create_event_queue);
	creater -> overrideNetworkQueue(&network_event_queue);
	creater -> overridStorageQueue(&storage_event_queue);

	network -> overrideInternalQueue(&network_event_queue);
	network -> overrideStorageQueue(&storage_event_queue);
	network -> overrideUploadQueue(&upload_event_queue);
	network -> overrideDesirializerQueue(&serverMsgProcessor_event_queue);
	
	sensor -> overrideInternalQueue(&sensor_event_queue);
	sensor -> overrideStorageQueue(&storage_event_queue);
	
	uploader -> overrideInternalQueue(&upload_event_queue);
	uploader -> overrideStorageQueue(&storage_event_queue);
	uploader -> overrideCreaterQueue(&create_event_queue);
		
	server_msg_processor -> overrideInternalQueue(&serverMsgProcessor_event_queue);
	server_msg_processor -> overrideStorageQueue(&storage_event_queue);
	
	eventMenager -> overrideInternalQueue(&event_queue);
	eventMenager -> overrideStorageQueue(&storage_event_queue);
	eventMenager -> overrideNetworkQueue(&network_event_queue);
	eventMenager -> overrideSensorQueue(&sensor_event_queue);
	eventMenager -> overrideUploadQueue(&upload_event_queue);
	eventMenager -> overrideCreateQueue(&create_event_queue);
	eventMenager -> overrideSerMsgProcessQueue(&serverMsgProcessor_event_queue);
	
	myNotif -> overrideInternalQueue(&notif_event_queue);
}