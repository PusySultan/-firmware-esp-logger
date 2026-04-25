/*
 * sensor_settings_t.hpp
 *
 *  Created on: 15 мар. 2026 г.
 *      Author: Kirill
 */

#ifndef MAIN_COMMONSTRUCT_GLOBAL_SETTINGS_T_HPP_
#define MAIN_COMMONSTRUCT_GLOBAL_SETTINGS_T_HPP_

#include <cstring>
#include <stdint.h>
#include "settings_const.hpp"
#include "secret.h"

struct global_settings_t
{
    // Единственный способ получить экземпляр
    static global_settings_t& getInstance()
    {
        static global_settings_t instance; // Создаётся один раз при первом вызове
        return instance;
    }
    
     // Удаляем конструктор копирования и оператор присваивания
    global_settings_t(const global_settings_t&) = delete;
    global_settings_t& operator=(const global_settings_t&) = delete;
	
	static const uint32_t INVENTOR_NUM_ADDR =	    0x000000; // - 0x000003 
	static const uint32_t FREQ_TEMP_SENSOR_1_ADDR = 0x000004; // - 0x000007 
	static const uint32_t FREQ_TEMP_SENSOR_2_ADDR = 0x000008; // - 0x00000B 
	static const uint32_t FREQ_TEMP_SENSOR_3_ADDR = 0x00000C; // - 0x00000F 
	static const uint32_t FREQ_TEMP_SENSOR_C_ADDR = 0x000010; // - 0x000013 
	static const uint32_t FREQ_DUST_SENSOR_1_ADDR = 0x000014; // - 0x000017 
	static const uint32_t FREQ_DUST_SENSOR_2_ADDR = 0x000018; // - 0x00001B
	static const uint32_t WIFI_NAME_ADDR = 			0x00001C; // - 0x00003B
	static const uint32_t WIFI_PASS_ADDR = 			0x00003C; // - 0x00007B
		
	uint32_t getSettingAddrByName(char* name)
	{
		if (strcmp(name, INVENTOR_NUM) == 0)  	   return INVENTOR_NUM_ADDR;
		if (strcmp(name, FREQ_TEMP_SENSOR_1) == 0) return FREQ_TEMP_SENSOR_1_ADDR;
		if (strcmp(name, FREQ_TEMP_SENSOR_2) == 0) return FREQ_TEMP_SENSOR_2_ADDR;
		if (strcmp(name, FREQ_TEMP_SENSOR_3) == 0) return FREQ_TEMP_SENSOR_3_ADDR;
		if (strcmp(name, FREQ_TEMP_SENSOR_C) == 0) return FREQ_TEMP_SENSOR_C_ADDR;
		if (strcmp(name, FREQ_DUST_SENSOR_1) == 0) return FREQ_DUST_SENSOR_1_ADDR;
		if (strcmp(name, FREQ_DUST_SENSOR_2) == 0) return FREQ_DUST_SENSOR_2_ADDR;
		if (strcmp(name, WIFI_NAME) == 0)  		   return WIFI_NAME_ADDR;
		if (strcmp(name, WIFI_PASS) == 0)  		   return WIFI_PASS_ADDR;
			
		return -1;
	}
	
	void getDefaultSettingByAddr(uint32_t addr, uint8_t* out_buffer)
	{
		switch (addr)
		{
			case FREQ_TEMP_SENSOR_1_ADDR: 
			{
				out_buffer[0] = 30;
				break;	
			};
			
			case FREQ_TEMP_SENSOR_2_ADDR: 
			{
				out_buffer[0] = 15;
				break;	
			};
			
			case FREQ_TEMP_SENSOR_3_ADDR: 
			{
				out_buffer[0] = 10;
				break;	
			};
			
			case FREQ_TEMP_SENSOR_C_ADDR: 
			{
				out_buffer[0] = 100;
				break;	
			};
			
			case FREQ_DUST_SENSOR_1_ADDR: 
			{
				out_buffer[0] = 100;
				break;	
			};
			
			case FREQ_DUST_SENSOR_2_ADDR: 
			{
				out_buffer[0] = 100;
				break;	
			};
			
			case WIFI_NAME_ADDR: 
			{
				memcpy(out_buffer, WIFI_DEFAULT_NAME, 64);
				break;	
			};
			
			case WIFI_PASS_ADDR: 
			{
				memcpy(out_buffer, WIFI_DEFAULT_PASS, 64);
				break;	
			};
			
		}
	}
	
	private:
	    // Приватный конструктор и деструктор запрещают создание извне
	    global_settings_t() = default;
	    ~global_settings_t() = default;
};

#endif /* MAIN_COMMONSTRUCT_GLOBAL_SETTINGS_T_HPP_ */
