/*
 * DustSensor.cpp
 *
 *  Created on: 6 апр. 2026 г.
 *      Author: Kirill
 */

#include "driver/gpio.h"
#include "DustSensor.hpp"
#include "esp_adc/adc_oneshot.h"
#include "freertos/FreeRTOS.h"
#include "global_settings_t.hpp"
#include "pins_config_const.hpp"
#include "sensors_const.hpp"

uint8_t DustSensor :: sensorCounter = 0;

DustSensor :: DustSensor (SensorsID id) : ISensor(id)
{
	sensorCounter++;
	setPinConfig();
	
	if(this -> sensorsAchieveLimit())
	{
		printf("Error check ADC conditions (DustSensor)\n");
		return;
	}
	
	if(adcInit() != ESP_OK)
	{
		printf("Error init ADC (DustSensor)\n");
		return;
	}
	
	svetPinInit();
	calibrateSensor();
}

DustSensor :: ~DustSensor ()
{
  adc_oneshot_del_unit(this -> adc_handle);
}

float DustSensor :: getData()
{
	int currentValue = 1;
	
	gpio_set_level(this -> LedPin, 1);
	vTaskDelay(pdMS_TO_TICKS(50));
	
	adc_oneshot_read(adc_handle, adc_chan_num, &currentValue);
	
	float retvalue = (((float)(maxValue - minValue)) / currentValue) * 100.0;  // %
	// printf("adc read: %i\n", currentValue);
	printf("caluculate dust value %f\n", retvalue);
	 
	return retvalue;
}

const char* DustSensor :: getType()
{
	return DUST_SENSOR_TYPE;
}

uint8_t DustSensor :: getInterval()
{
	return this -> interval;	
}

void DustSensor :: setInterval(uint32_t s)
{
	this -> interval = s;
}

esp_err_t DustSensor :: adcInit()
{
	esp_err_t err;
	
	this -> adc_comm_config = {
		.unit_id = ADC_UNIT_1,				// АЦП №1
		.ulp_mode = ADC_ULP_MODE_DISABLE,	// ULP mode is not used in one-shot mode,
	};
	
	this -> adc_chan_config = {
		 .atten = ADC_ATTEN_DB_12, 			// диапазон 0 -3.3
         .bitwidth = ADC_BITWIDTH_DEFAULT,	// 12 бит разрешение 
	};
	
	err = adc_oneshot_new_unit(&adc_comm_config, &adc_handle);
	
	if(err != ESP_OK)
	{
		printf("Error init ADC common\n");
		return err;	
	}
	
	err = adc_oneshot_config_channel(adc_handle, adc_chan_num,&adc_chan_config);
	if(err != ESP_OK)
	{
		printf("Error init ADC chanel\n");
		return err;	
	}
	
	return ESP_OK;
}

esp_err_t DustSensor :: svetPinInit()
{
	esp_err_t err;
	
	err = gpio_set_direction(this -> LedPin, GPIO_MODE_OUTPUT);
	if(err != ESP_OK)
	{
		return err;
	}
	
	err = gpio_set_level(this -> LedPin, 0);
	if(err != ESP_OK)
	{
		return err;
	}
	
	return ESP_OK;
	
}

void  DustSensor :: calibrateSensor()
{
	gpio_set_level(this -> LedPin, 0);
	vTaskDelay(pdMS_TO_TICKS(50));
	
	adc_oneshot_read(adc_handle, adc_chan_num, &this -> minValue);
	
	gpio_set_level(this -> LedPin, 1);
	vTaskDelay(pdMS_TO_TICKS(50));
	
	adc_oneshot_read(adc_handle, adc_chan_num, &this -> maxValue);
	
	printf("min value: %i, max value: %i\n", minValue, maxValue);	
}

bool  DustSensor :: sensorsAchieveLimit()
{
	return sensorCounter >= DUST_SENSOR_LINIT;
}

void  DustSensor :: setPinConfig()
{
	if(sensorCounter  == 1)
	{
		this -> LedPin = DUST_LED_1_PIN;
		this -> adc_chan_num = DUST_1_ADC_CHANEL;
		
		return;
	}
	
	this -> LedPin = DUST_LED_2_PIN;
	this -> adc_chan_num = DUST_2_ADC_CHANEL;
}