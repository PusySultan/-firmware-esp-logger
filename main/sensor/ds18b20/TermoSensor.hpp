/*
 * TermoSensor.hpp
 *
 *  Created on: 15 мар. 2026 г.
 *      Author: Kirill
 */

#ifndef MAIN_SENSOR_DS18B20_TERMOSENSOR_HPP_
#define MAIN_SENSOR_DS18B20_TERMOSENSOR_HPP_

#include "ISensor.hpp"
#include "SensorTypes.hpp"
#include "onewire_bus.h"

class TermoSensor : public ISensor
{
	public:
	  TermoSensor (SensorsID id);
      virtual ~TermoSensor();
  		
  	  float getData() override;
	  const char* getType() override;
	  	
	  uint8_t getInterval() override;
      void setInterval(uint32_t s) override;
      	
      SensorsID getSensorID();
      
    private:
    
    	uint8_t interval = 60;

		uint8_t read_cmd[2] = {0xCC, 0xBE};
		uint8_t convert_cmd[2] = {0xCC, 0x44};
  		uint8_t read_buf[2] = {};
  		int16_t temp_raw = 0;
  		
  		onewire_bus_handle_t bus;
		onewire_bus_config_t bus_config;
		onewire_bus_rmt_config_t rmt_config;

  		float ds18b20_read_temp(void);
  		uint8_t ds18b20_getAddress(void);
  		bool checkDeviceOnWire();
		bool ds18b20_convert_temp(void);
};

#endif /* MAIN_SENSOR_DS18B20_TERMOSENSOR_HPP_ */
