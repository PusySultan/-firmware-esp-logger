/*
 * collection_t.hpp
 *
 *  Created on: 8 апр. 2026 г.
 *      Author: Kirill
 */

#ifndef MAIN_COMMONSTRUCT_COLLECTIONFUNC_HPP_
#define MAIN_COMMONSTRUCT_COLLECTIONFUNC_HPP_

#include <cstring>
#include "collection_t.hpp"

inline char* collection_to_string(collection_t coll)
{
    switch (coll)
    {
        case DEVICE_TEMP:  return "DEVICE_TEMP";
        case DEVICE_DUST:  return "DEVICE_DUST";
        case DEVICE_EVENT: return "DEVICE_EVENT";
        case SETTING:      return "SETTING";
        default:           return "UNKNOWN";
    }
}

inline collection_t string_to_collection(const char* name)
{
    if (name == nullptr) return UNKNOW_COLLECTION;
    if (strcmp(name, "DeviceTemp") == 0)  return DEVICE_TEMP;
    if (strcmp(name, "DeviceDustiness") == 0)  return DEVICE_DUST;
    if (strcmp(name, "DeviceEvent") == 0) return DEVICE_EVENT;
    if (strcmp(name, "Setting") == 0)      return SETTING;
    return UNKNOW_COLLECTION;
}


#endif /* MAIN_COMMONSTRUCT_COLLECTIONFUNC_HPP_ */
