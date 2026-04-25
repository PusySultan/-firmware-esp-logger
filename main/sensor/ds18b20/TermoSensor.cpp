/*
 * TermoSensor.cpp
 *
 *  Created on: 15 мар. 2026 г.
 *      Author: Kirill
 */

#include <cmath>
#include "TermoSensor.hpp"
#include "SensorTypes.hpp"
#include "freertos/FreeRTOS.h"
#include "pins_config_const.hpp"
#include "settings_const.hpp"

TermoSensor :: TermoSensor (SensorsID id)
			 : ISensor(id)
{
	uint8_t pin = id == TEMP_SENSOR_1_ID ? TEMP_SENSOR_1_PIN : 
				  id == TEMP_SENSOR_2_ID ? TEMP_SENSOR_2_PIN : TEMP_SENSOR_3_PIN;
	
	// install new 1-wire bus
    this -> bus_config =
    {
        .bus_gpio_num = pin,
        .flags = {
            .en_pull_up = 1, // enable internal pull-up resistor
        }
    };
    
    this -> rmt_config =
    {
        .max_rx_bytes = 8, // 1byte ROM command + 8byte ROM number + 1byte device command
    };
    
    onewire_new_bus_rmt(&bus_config, &rmt_config, &bus);
}

TermoSensor::~TermoSensor ()
{
	// Удаляем шину
	onewire_bus_del(bus);
}

float TermoSensor :: getData()
{
	if(!checkDeviceOnWire()) 	return NAN;
	if(!ds18b20_convert_temp()) return NAN;
	
	vTaskDelay(pdMS_TO_TICKS(1000)); 

	return ds18b20_read_temp();
}

const char* TermoSensor :: getType()
{
	return TEMP_SENSOR_TYPE;
}

uint8_t TermoSensor :: getInterval()
{
	return this ->  interval;
}

void TermoSensor :: setInterval(uint32_t s)
{
	this -> interval = s;
}

SensorsID TermoSensor :: getSensorID()
{
	return this -> ID;
}

// -- private methodes:

bool TermoSensor :: checkDeviceOnWire()
{
	if(onewire_bus_reset(bus) != ESP_OK)
	{
		return false;
	}
	
	return true;
}

float TermoSensor :: ds18b20_read_temp(void)
{	
	if(!checkDeviceOnWire()) return NAN;
	onewire_bus_write_bytes(bus, read_cmd, 2);
	    
	onewire_bus_read_bytes(bus, read_buf, 2);
	    
	temp_raw = (read_buf[1] << 8) | read_buf[0];
    
    return temp_raw / 16.0f;
}

bool TermoSensor :: ds18b20_convert_temp(void)
{
	if(onewire_bus_reset(bus) != ESP_OK)
	{
		return false;
	}
    
	onewire_bus_write_bytes(bus, convert_cmd, 2);
	return true;
}

