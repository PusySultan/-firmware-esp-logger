/*
 * SectorAddr.hpp
 *
 *  Created on: 19 мар. 2026 г.
 *      Author: Kirill
 */

#ifndef MAIN_COMMONSTRUCT_ADDRESFUNC_HPP_
#define MAIN_COMMONSTRUCT_ADDRESFUNC_HPP_

#include <map>
#include <stdint.h>
#include "SensorTypes.hpp"

inline std::map<SensorsID, std::pair<uint32_t, uint32_t>> sector_addr_t
{
		   // id			    // min    // max
	  {TEMP_SENSOR_1_ID,    {0x060000, 0x447FFF}},
	  {TEMP_SENSOR_2_ID,    {0x448000, 0x82FFFF}},
	  {TEMP_SENSOR_3_ID,    {0x830000, 0xC17FFF}},
	  {TEMP_SENSOR_C_ID,    {0x006000, 0x05FFFF}},
	  {DUST_SENSOR_1_ID,    {0xC18000, 0xE0BFFF}},
	  {DUST_SENSOR_2_ID, 	 {0xE0C000, 0xFFFFFF}},
};

inline SensorsID getSensorIdByAddr(uint32_t addr)
{
	std::map<SensorsID, std::pair<uint32_t, uint32_t>>::iterator it = sector_addr_t.begin();
	
	while(it != sector_addr_t.end())
	{
    	if(addr >= it -> second.first && addr <= it -> second.second)
    	{
			return it -> first;
		}
    	
		it++;
	}
	
	return UNKNOWN_SENSOR;
} 


#endif /* MAIN_COMMONSTRUCT_ADDRESFUNC_HPP_ */
