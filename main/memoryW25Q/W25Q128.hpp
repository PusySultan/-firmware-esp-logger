/*
 * W25Q128.h
 *
 *  Created on: 11 февр. 2026 г.
 *      Author: Kirill
 */
 
 #pragma once

#ifndef MAIN_MEMORYW25Q_W25Q128_HPP_
#define MAIN_MEMORYW25Q_W25Q128_HPP_

	#include <cstring>
	#include "StorageTypes.hpp"
	#include "driver/spi_master.h"
	
	#define SPI_HOST_ID SPI2_HOST

class W25Q128
{
	public:
 	 	W25Q128 ();
 	 	virtual	~W25Q128 ();
  		
  		void readByAddr(uint8_t* readingData, uint32_t addr, int length);
  		void writeByAddr(uint8_t* data, uint32_t addr, int length);
  		void writeTransaction(block_data elements[], uint8_t trans_size, uint32_t sector_addr);
  		
  		void eraseSector(uint32_t addr);
  		uint8_t getMemoryStatus_1();
  		uint8_t getMemoryStatus_2();
  		
  		SemaphoreHandle_t semaphore;
  		
  	private:
  	
		spi_bus_config_t busConfig;					// конфиг шины
		spi_device_handle_t spi_device;				// SPI устройство
		spi_device_interface_config_t deviceConfig;	// Конфиги устройства
		
  		void w25MemoryInit();
		void initSPI();
		void writeEnable();
};

#endif /* MAIN_MEMORYW25Q_W25Q128_HPP_ */
