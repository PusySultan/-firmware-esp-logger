/*
 * DateTime.hpp
 *
 *  Created on: 25 мар. 2026 г.
 *      Author: Kirill
 */

#ifndef MAIN_DATETIME_DATETIME_HPP_
#define MAIN_DATETIME_DATETIME_HPP_

#include <cstdio>
#include <cstdlib>
#include <cstring>

struct DateTime
{
	uint8_t seconds;
	uint8_t minutes; 
	uint8_t hours;
	uint8_t date;
	uint8_t month;
	uint8_t day;
	uint8_t year;

     /**
     * Возвращает абсолютную разницу в секундах между текущей датой и другой.
     * @param other другая дата
     * @return количество секунд в дате
     * @note Работает корректно для любых дат в пределах до 28 дней (месяца)
     */
    uint64_t toSecondsShort()
    {
        return seconds + (minutes * 60) + (hours * 60 * 60) + (date * 24 * 60 * 60);
    }

	void toString(char* buffer)
	{
	    snprintf(buffer, 32, "%04d-%02d-%02d %02d:%02d:%02d",
	             year + 2000, month, date, hours, minutes, seconds);
	}
	
	void toBytes(uint8_t* out) const
	{
        memcpy(out, this, sizeof(*this));
    }
    
    void fromBytes(const uint8_t* in)
    {
        memcpy(this, in, sizeof(*this));
    }
    
    static DateTime fromStringStatic(char dtStr[19])
    {
		// 2026-04-12 14:08:49
		DateTime dt;
		
		uint16_t year, month, date, hours, minutes, seconds;
		
		if (sscanf(dtStr, "%hd-%hd-%hd %hd:%hd:%hd",
           &year, &month, &date, &hours, &minutes, &seconds) != 6)
        {
	        dt = {};
	        return dt;
    	}
    
    	dt.year = static_cast<uint8_t>(year - 2000);
	    dt.month = static_cast<uint8_t>(month);
	    dt.date = static_cast<uint8_t>(date);
	    dt.hours = static_cast<uint8_t>(hours);
	    dt.minutes = static_cast<uint8_t>(minutes);
	    dt.seconds = static_cast<uint8_t>(seconds);
		
		return dt;
	}
    
    static DateTime fromBytesStatic(const uint8_t* in)
    {
        DateTime dt;
        memcpy(&dt, in, sizeof(dt));
        return dt;
    }
} __attribute__((packed));

#endif /* MAIN_DATETIME_DATETIME_HPP_ */
