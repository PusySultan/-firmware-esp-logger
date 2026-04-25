/*
 * wifi_settings_t.hpp
 *
 *  Created on: 14 мар. 2026 г.
 *      Author: Kirill
 */

#ifndef MAIN_COMMONSTRUCT_WIFI_SETTINGS_T_HPP_
#define MAIN_COMMONSTRUCT_WIFI_SETTINGS_T_HPP_

#include <sys/_stdint.h>

struct wifi_settings_t
{
	uint8_t wifi_name[32];
	uint8_t wifi_pass[64];
};

#endif /* MAIN_COMMONSTRUCT_WIFI_SETTINGS_T_HPP_ */
