/*
 * ISensor.hpp
 *
 *  Created on: 2 мар. 2026 г.
 *      Author: Kirill
 */

#ifndef MAIN_SENSOR_ISENSOR_HPP_
#define MAIN_SENSOR_ISENSOR_HPP_

#include "nvs.h"
#include "SensorTypes.hpp"

class ISensor
{
	public:
	  
	  ISensor (SensorsID id);
	  virtual ~ISensor ();
	  
	  virtual float getData() { return 0.0f; }
      virtual const char* getType() { return "ISensor"; }
      virtual uint8_t getInterval() { return 1; }
      virtual void setInterval(uint32_t) { /* ничего не делаем */ }
      
      uint32_t getPointer();
      void savePointer();
      SensorsID getSensorID();
      
    protected:
      SensorsID ID;
      nvs_handle_t nvs_handle;
      uint32_t curent_pointer = UINT32_MAX;
            
      void calculatePointer();
};

#endif /* MAIN_SENSOR_ISENSOR_HPP_ */
