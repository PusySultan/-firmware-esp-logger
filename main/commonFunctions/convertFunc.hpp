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

// === Конвертация uint64_t -> uint8_t[8] (Little-Endian) ===
inline void writeU64LE(uint8_t* outData, uint64_t value)
{
    outData[0] = (uint8_t)(value & 0xFF);
    outData[1] = (uint8_t)((value >> 8) & 0xFF);
    outData[2] = (uint8_t)((value >> 16) & 0xFF);
    outData[3] = (uint8_t)((value >> 24) & 0xFF);
    outData[4] = (uint8_t)((value >> 32) & 0xFF);
    outData[5] = (uint8_t)((value >> 40) & 0xFF);
    outData[6] = (uint8_t)((value >> 48) & 0xFF);
    outData[7] = (uint8_t)((value >> 56) & 0xFF);
}

// === Конвертация uint8_t[8] -> uint64_t (Little-Endian) ===
inline uint64_t readU64LE(const uint8_t* inData)
{
    return ((uint64_t)inData[0])      |
           ((uint64_t)inData[1] << 8)  |
           ((uint64_t)inData[2] << 16) |
           ((uint64_t)inData[3] << 24) |
           ((uint64_t)inData[4] << 32) |
           ((uint64_t)inData[5] << 40) |
           ((uint64_t)inData[6] << 48) |
           ((uint64_t)inData[7] << 56);
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
