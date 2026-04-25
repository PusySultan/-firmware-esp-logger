/*
 * MulticastDNS.cpp
 *
 *  Created on: 12 февр. 2026 г.
 *      Author: Kirill
 */

#include "mdns.h"
#include "lwip/ip_addr.h"
#include "MulticastDNS.hpp"

MulticastDNS::MulticastDNS ()
{
}

MulticastDNS::~MulticastDNS ()
{
	mdns_free();
}

esp_err_t MulticastDNS :: initMDNS()
{
	esp_err_t err;;
	
	if ((err = mdns_init()) != ESP_OK)
    {
        return err;
    }
    
    return ESP_OK;   
}

esp_err_t MulticastDNS :: findServerURL()
{
	mdns_result_t* results;
	esp_err_t err;
		
	// Ищем конкретный экземпляр имя - _myservice тип - _tcp
	err = mdns_query_ptr("_myservice", "_tcp", 3500, 20, &results);
;
	if (err != ESP_OK)
	{
		return err;
	}
	
	while(results)
	{
		if(!(results -> addr && results -> addr -> addr.type == IPADDR_TYPE_V4))
		{
			results = results -> next;
		    continue;
		}
				
		char ipstr[16] = {};
		
		esp_ip4addr_ntoa(
			&results -> addr -> addr.u_addr.ip4, 
			ipstr, 
	 	    sizeof(ipstr));
	  
		sprintf(this -> server_url, "ws://%s:8080/controller", ipstr);
				
	  	break;
	}
	
	mdns_query_results_free(results);
	return checkURL();
	
}

esp_err_t MulticastDNS :: checkURL()
{
	
	if(this -> server_url[0] == 0)
	{
		return ESP_ERR_INVALID_RESPONSE;
	}

	return ESP_OK;
}

char* MulticastDNS :: getServerUrl()
{
	return server_url;
}
