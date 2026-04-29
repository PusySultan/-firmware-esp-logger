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
	
	// Преобразование даты в количество секунд с начала эпохи 2000-01-01 00:00:00
    uint64_t toSeconds() const
	{
        const uint64_t SECONDS_PER_DAY = 86400;
        uint64_t days = daysSinceEpoch(getFullYear(), month, date);
        uint64_t secondsToday = seconds + minutes * 60u + hours * 3600u;
        return days * SECONDS_PER_DAY + secondsToday;
    }

	/**
     * Возвращает абсолютную разницу в секундах между текущей датой и другой.
     * @param other другая дата
     * @return количество секунд между датами (всегда неотрицательное)
     * @note Работает корректно для любых дат в пределах эпохи,
     *       но предполагает, что разница не превышает ~584 миллиона лет (в пределах uint64_t).
     *       Для вашего случая с year в uint8_t разница максимум 255 лет — вполне безопасно.
     */
    uint64_t getDifferenceAsSeconds(const DateTime& other) const
	{
        uint64_t selfSec = this -> toSeconds();
        uint64_t otherSec = other.toSeconds();

        return (selfSec >= otherSec) ? (selfSec - otherSec) : (otherSec - selfSec);
    }

	uint16_t getFullYear() const {
        return 2000 + year;
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
    
	// Проверка, является ли год високосным (григорианский календарь)
    static bool isLeapYear(uint16_t fullYear) {
        return (fullYear % 4 == 0) && (fullYear % 100 != 0 || fullYear % 400 == 0);
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

	// Количество дней в месяце (с учётом високосности)
    static uint8_t daysInMonth(uint16_t fullYear, uint8_t month)
	{
        static const uint8_t days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

        if (month == 2 && isLeapYear(fullYear)) return 29;
        return days[month - 1];
    }

	 // Количество дней от эпохи 2000-01-01 до заданной даты (не включая текущий день полностью)
    static uint64_t daysSinceEpoch(uint16_t fullYear, uint8_t month, uint8_t day)
	{
        uint64_t days = 0;

        // Суммируем дни за полные годы от 2000 до года перед fullYear
        for (uint16_t y = 2000; y < fullYear; ++y)
		{
            days += isLeapYear(y) ? 366 : 365;
        }
        // Суммируем дни за полные месяцы текущего года
        for (uint8_t m = 1; m < month; ++m)
		{
            days += daysInMonth(fullYear, m);
        }
        
		// Добавляем дни в текущем месяце (минус 1, потому что сегодняшний день ещё не закончился)
        days += (day - 1);
        return days;
    }
} __attribute__((packed));

#endif /* MAIN_DATETIME_DATETIME_HPP_ */
