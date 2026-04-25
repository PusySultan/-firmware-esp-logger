/*
 * DateTimeSensor.hpp
 *
 *  Created on: 25 мар. 2026 г.
 *      Author: Kirill
 */

#ifndef MAIN_DATETIME_DATETIMESENSOR_HPP_
#define MAIN_DATETIME_DATETIMESENSOR_HPP_

// Регистры DS1302
#include "soc/gpio_num.h"
#include "DateTime.hpp"

class DateTimeSensor final
{
	public:
	  
	  // Ссылка на единственный обьект этого класса (тот что внутри static блока)
	  static DateTimeSensor& getInstance()
	  {
      	static DateTimeSensor instance;
        return instance;
      }
      
      	void ds1302_init(uint8_t ena, uint8_t clk, uint8_t dat);
		void ds1302_getDateTime(DateTime *dt);
		void ds1302_setDateTime(DateTime *dt);
	  
	private:
	
	  	DateTimeSensor ();
	  	virtual ~DateTimeSensor ();
			
		gpio_num_t pin_ena;
		gpio_num_t pin_clk;
		gpio_num_t pin_dat;
	
		uint8_t ds1302_readByte();
		void ds1302_write_byte(uint8_t data);
		void nextBit();
		
		uint8_t ds1302_dec2bcd(uint8_t dec);
		uint8_t ds1302_bcd2dec(uint8_t bcd);
		
		void prepareRead(uint8_t address);
		void prepareWrite(uint8_t address);
		
	    static constexpr uint8_t DS1302_SECONDS   = 0x80;
	    static constexpr uint8_t DS1302_MINUTES   = 0x82;
	    static constexpr uint8_t DS1302_HOURS     = 0x84;
	    static constexpr uint8_t DS1302_DATE      = 0x86;
	    static constexpr uint8_t DS1302_MONTH     = 0x88;
	    static constexpr uint8_t DS1302_DAY       = 0x8A;
	    static constexpr uint8_t DS1302_YEAR      = 0x8C;
	    static constexpr uint8_t DS1302_REG_WP    = 0x8E;
	    static constexpr uint8_t DS1302_REG_BURST = 0xBE;
};

#endif /* MAIN_DATETIME_DATETIMESENSOR_HPP_ */
