/*
 * DustSensor.hpp
 *
 *  Created on: 6 апр. 2026 г.
 *      Author: Kirill
 */

#ifndef MAIN_SENSOR_DUSTSENSOR_DUSTSENSOR_HPP_
#define MAIN_SENSOR_DUSTSENSOR_DUSTSENSOR_HPP_

#include "ISensor.hpp"
#include "esp_adc/adc_oneshot.h"
#include "soc/gpio_num.h"

class DustSensor : public ISensor
{
	public:
		DustSensor (SensorsID id);
	  	virtual ~DustSensor ();
	  	
	  	float getData() override;
	  	const char* getType() override;
	  	uint8_t getInterval() override;
      	void setInterval(uint32_t s) override;
    
    private:
    	static uint8_t sensorCounter;
    
    	int minValue = 0;
    	int maxValue = 0;
    	
    	uint32_t interval = 60;
    	
    	gpio_num_t LedPin;
		adc_channel_t adc_chan_num;
    	adc_oneshot_unit_handle_t   adc_handle;
    	adc_oneshot_unit_init_cfg_t adc_comm_config;
    	adc_oneshot_chan_cfg_t		adc_chan_config;
    	
    	esp_err_t adcInit();
    	esp_err_t svetPinInit();
    	
    	void setPinConfig();
    	void calibrateSensor();
    	bool sensorsAchieveLimit();
};

#endif /* MAIN_SENSOR_DUSTSENSOR_DUSTSENSOR_HPP_ */
