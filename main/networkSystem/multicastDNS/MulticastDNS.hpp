/*
 * MulticastDNS.hpp
 *
 *  Created on: 12 февр. 2026 г.
 *      Author: Kirill
 */

#ifndef MAIN_MULTICASTDNS_MULTICASTDNS_HPP_
#define MAIN_MULTICASTDNS_MULTICASTDNS_HPP_

#include "esp_err.h"

class MulticastDNS
{
	public:
	  MulticastDNS ();
	  virtual ~MulticastDNS ();
	  
	  esp_err_t initMDNS();
	  esp_err_t findServerURL();
	  esp_err_t checkURL();
	  
	  char* getServerUrl();
	  
	private:
	  char server_url[64] = {};
};

#endif /* MAIN_MULTICASTDNS_MULTICASTDNS_HPP_ */
