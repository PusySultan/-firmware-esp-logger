/*
 * EventMenager.hpp
 *
 *  Created on: 13 апр. 2026 г.
 *      Author: Kirill
 */

#ifndef MAIN_EVENTMENAGER_EVENTMENAGER_HPP_
#define MAIN_EVENTMENAGER_EVENTMENAGER_HPP_

#include <map>
#include <functional>
#include "CaseOpeningEvent.hpp"
#include "DateTime.hpp"
#include "EventTypes.hpp"
#include "IEvent.hpp"
#include "SensorTypes.hpp"
#include "freertos/FreeRTOS.h"

class EventMenager final
{
	using func = std::function<void(event_cmd_t*)>;
	
	public:
		EventMenager ();
		virtual ~EventMenager ();
		
		void eventProcessor(event_cmd_t*);
		
		void static overrideInternalQueue(QueueHandle_t* queue);
		void static overrideStorageQueue(QueueHandle_t* queue);
		void static overrideSensorQueue(QueueHandle_t* queue);
		void static overrideUploadQueue(QueueHandle_t* queue);
		void static overrideNetworkQueue(QueueHandle_t* queue);
		void static overrideCreateQueue(QueueHandle_t* queue);
		void static overrideSerMsgProcessQueue(QueueHandle_t* queue);

		IEvent* caseOpeningEvent;

		bool getKillStatus() {return this -> KILL_PROCESS_FLSG;}
			  		
	private:
		static QueueHandle_t* event_queue;
		static QueueHandle_t* storage_event_queue;
		static QueueHandle_t* sensor_event_queue;
		static QueueHandle_t* upload_event_queue;
		static QueueHandle_t* network_event_queue;
		static QueueHandle_t* create_event_queue;
		static QueueHandle_t* serverMsgProcessor_event_queue;
	
		CaseOpeningEvent* _caseOpeningEvent;
		IEvent* _voltageOffEvent;

		DateTime enebleTime;
		DateTime disableTime;
		DateTime caseOpenTime;

		bool KILL_PROCESS_FLSG = false;

		std::map<event_type_t, func> eventProcessors;

		void connectGND();
		void fillFunctionMap();
	
		void initNetwork();
		void createSensor(SensorsID);

		void killStorage();
		void killNetwork();
		void initUpload();
		void killUpload();
		void killSensors();
		void killCreater();
		void killMsgProcessor();
	
		void saveEnableEvent();
		void saveDisabeEvent();
		void saveJobIntervalTime();
		void saveTotalJobTime();

		uint64_t getTotalJobTime();
};

#endif /* MAIN_EVENTMENAGER_EVENTMENAGER_HPP_ */
