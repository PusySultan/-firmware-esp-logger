/*
 * request_type_func.hpp
 *
 *  Created on: 19 апр. 2026 г.
 *      Author: Kirill
 */

#ifndef MAIN_COMMONFUNCTIONS_REQUEST_TYPE_FUNC_HPP_
#define MAIN_COMMONFUNCTIONS_REQUEST_TYPE_FUNC_HPP_

#include "request_type_t.hpp"
#include <cstring>

inline request_type_t string_to_requestType(const char* request_type_str)
{
	if (strcmp(request_type_str, "GET") == 0)  return GET;
	if (strcmp(request_type_str, "POST") == 0)  return POST;
	
	return UNKNOW_TYPE; 
}

#endif /* MAIN_COMMONFUNCTIONS_REQUEST_TYPE_FUNC_HPP_ */
