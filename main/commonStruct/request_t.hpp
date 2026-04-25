/*
 * request_t.hpp
 *
 *  Created on: 18 апр. 2026 г.
 *      Author: Kirill
 */

#ifndef MAIN_COMMONSTRUCT_REQUEST_T_HPP_
#define MAIN_COMMONSTRUCT_REQUEST_T_HPP_

#include "cJSON.h"
#include "collection_t.hpp"
#include "request_type_t.hpp"

struct request_t
{
	request_type_t request_type;
	collection_t   collection;
	cJSON* message;
};

#endif /* MAIN_COMMONSTRUCT_REQUEST_T_HPP_ */
