/*
 * convertStrruct.hpp
 *
 *  Created on: 21 мар. 2026 г.
 *      Author: Kirill
 */

#ifndef MAIN_COMMONSTRUCT_CONVERTFUNCTIONS_HPP_
#define MAIN_COMMONSTRUCT_CONVERTFUNCTIONS_HPP_

#include <sys/_stdint.h>

// === Конвертация uint32_t -> uint8_t[4] (Little-Endian) ===
inline void writeU32LE(uint8_t* outData, uint32_t value)
{
    outData[0] = value & 0xFF;         // Байт 0: младший
    outData[1] = (value >> 8) & 0xFF;  // Байт 1
    outData[2] = (value >> 16) & 0xFF; // Байт 2
    outData[3] = (value >> 24) & 0xFF; // Байт 3: старший
}

// === Конвертация uint8_t[4] -> uint32_t (Little-Endian) ===
inline uint32_t readU32LE(const uint8_t* inData)
{
    return ((uint32_t)inData[0]) |
           ((uint32_t)inData[1] << 8) |
           ((uint32_t)inData[2] << 16) |
           ((uint32_t)inData[3] << 24);
}

inline bool arrayContainsTrush(uint8_t* array, uint8_t size)
{
    for(int i = 0; i < size; i++)
    {
        if(array[i] != 0xFF)   // Если есть хоть один байт != 0xFF
            return false;      // Данные валидны
    }
    return true;  // Все байты 0xFF → "мусор"
}

#endif /* MAIN_COMMONSTRUCT_CONVERTFUNCTIONS_HPP_ */
