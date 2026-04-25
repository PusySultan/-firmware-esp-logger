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
	
	DateTime operator+ (const DateTime& other) {
		
		DateTime dt {};
		
		dt.year    += other.year;
		dt.month   += other.month;
		dt.day     += other.day;
		dt.date    += other.date;
		dt.hours   += other.hours;
		dt.minutes += other.minutes;
		dt.seconds += other.seconds;
		
		return  dt; 
	}
	
	DateTime operator- (const DateTime& other) {
		
		DateTime dt {};
		
		dt.year    -= other.year;
		dt.month   -= other.month;
		dt.day     -= other.day;
		dt.date    -= other.date;
		dt.hours   -= other.hours;
		dt.minutes -= other.minutes;
		dt.seconds -= other.seconds;
		
		return  dt; 
	}

	void operator-= (const DateTime& other) {
		this -> year    -= other.year;
		this -> month   -= other.month;
		this -> day     -= other.day;
		this -> date    -= other.date;
		this -> hours   -= other.hours;
		this -> minutes -= other.minutes;
		this -> seconds -= other.seconds; 
	}

	void operator+= (const DateTime& other) {
		this -> year    += other.year;
		this -> month   += other.month;
		this -> day     += other.day;
		this -> date    += other.date;
		this -> hours   += other.hours;
		this -> minutes += other.minutes;
		this -> seconds += other.seconds; 
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
