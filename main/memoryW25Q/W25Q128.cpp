/*
 * W25Q128.cpp
 *
 *  Created on: 11 февр. 2026 г.
 *      Author: Kirill
 */

#include "W25Q128.hpp"
#include "pins_config_const.hpp"
#include "soc/gpio_num.h"

W25Q128::W25Q128 ()
{
	initSPI();
	w25MemoryInit();

	
	this -> semaphore = xSemaphoreCreateBinary();  	
  	xSemaphoreGive(this -> semaphore);
}

W25Q128::~W25Q128 ()
{
}

void W25Q128 :: w25MemoryInit()
{
    this -> deviceConfig = {
		.command_bits = 0,              // Биты команды (0 если команда в данных)
		.address_bits = 0,              // Биты адреса (0 если адрес в данных)
		.dummy_bits = 0,                // Dummy биты
		.mode = 0,                      // Режим SPI 0
		.cs_ena_pretrans = 0,           // Активация CS перед передачей (в тактах)
	    .cs_ena_posttrans = 0,          // Держать CS после передачи (в тактах)
	    .clock_speed_hz = 1000000,      // Частота 1 МГц)
	    .spics_io_num = MEMORY_PIN_NUM_CS,     // GPIO для Chip Select
	    .flags = 0,                     // Флаги устройства
	   	.queue_size = 20,               // Размер очереди транзакций
	};
    
	// добавляем устройство на шину
	spi_bus_add_device(SPI_HOST_ID, &deviceConfig, &spi_device);	
}

void W25Q128 :: initSPI()
{
    this -> busConfig = {
        .mosi_io_num = MEMORY_PIN_NUM_MOSI,
        .miso_io_num = MEMORY_PIN_NUM_MISO,
        .sclk_io_num = MEMORY_PIN_NUM_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4096,
        .flags = 0,
        .intr_flags = 0
    };
    
    spi_bus_initialize(SPI_HOST_ID, &busConfig, SPI_DMA_CH_AUTO);
}

void W25Q128 :: writeTransaction(block_data elements[], uint8_t trans_size, uint32_t sector_addr)
{
	eraseSector(sector_addr);
	
	for(uint8_t i = 0; i < trans_size; i++)
	{
		writeByAddr(elements[i].data, elements[i].addr, elements[i].length);
	}	
}

void W25Q128 :: writeByAddr(uint8_t* data, uint32_t addr, int length)
{
	writeEnable();
	
    uint8_t* cmd = (uint8_t*) heap_caps_aligned_alloc(32, length + 4, MALLOC_CAP_DMA);

    if (cmd == NULL)
    {
        return;
    }
     
    cmd[0] = 0x02;                    // Page Program command
    cmd[1] = (addr >> 16) & 0xFF;
    cmd[2] = (addr >> 8) & 0xFF;
    cmd[3] = addr & 0xFF;
    
    // копируем данные в буфер по адресу с 4 элемента
    memcpy(&cmd[4], data, length);
    
    spi_transaction_t writeTrans = {
        .flags = 0,
        .length = static_cast<size_t>((length + 4) * 8),	// бит
        .rxlength = 0,
        .tx_buffer = cmd,
        .rx_buffer = NULL
    };

    spi_device_transmit(this -> spi_device, &writeTrans);
    heap_caps_free(cmd);
            
    // 3. Ждем завершения записи
    uint8_t status;
    int timeout = 1000; // 1000 попыток
    
    esp_rom_delay_us(15);
    
    do
    {
        status = getMemoryStatus_1();
       	esp_rom_delay_us(1);
        timeout--;

        if (timeout <= 0)
        {
			printf("bad memrory status\n");
            break;
        }
    }
    
    while(status & 0x01);
}

void W25Q128 :: readByAddr(uint8_t* data, uint32_t addr, int length)
{
	// Выделяем память выровненного размера
    uint8_t* tx_buffer = (uint8_t*) heap_caps_aligned_alloc(32, 128, MALLOC_CAP_DMA);
    uint8_t* rx_buffer = (uint8_t*) heap_caps_aligned_alloc(32, 128, MALLOC_CAP_DMA);
    
    if (!tx_buffer || !rx_buffer)
    {
        if (tx_buffer) heap_caps_free(tx_buffer);
        if (rx_buffer) heap_caps_free(rx_buffer);
        return;
    }

    tx_buffer[0] = 0x03;
    tx_buffer[1] = (addr >> 16) & 0xFF;
    tx_buffer[2] = (addr >> 8) & 0xFF;
    tx_buffer[3] = addr & 0xFF;
    
    for(int i = 4; i < length + 4; i++)
    {
        tx_buffer[i] = 0xFF;
    }
    
    spi_transaction_t readDataByAddrTransaction = {
        .flags = 0,
        .length =  1024,
        .rxlength = 1024,
        .tx_buffer = tx_buffer,
        .rx_buffer = rx_buffer
    };
    
    
    spi_device_transmit(this -> spi_device, &readDataByAddrTransaction);
    	
	// Копируем только данные
    memcpy(data, &rx_buffer[4], length);

    heap_caps_free(tx_buffer);
    heap_caps_free(rx_buffer);
}

void W25Q128 :: eraseSector(uint32_t addr)
{
	// Включаем запись перед стиранием
	writeEnable();
	
    // Команда полного стирания сектора
   uint8_t* cmd = (uint8_t*) heap_caps_aligned_alloc(32, 4, MALLOC_CAP_DMA);
    
    cmd[0] = 0x20;
    cmd[1] = (addr >> 16) & 0xFF;
    cmd[2] = (addr >> 8) & 0xFF;
    cmd[3] = addr & 0xFF; 
    
    spi_transaction_t eraseSectorTransaction = {
        .flags = 0,
        .length = 32,
        .rxlength = 0,
        .tx_buffer = cmd,
        .rx_buffer = NULL
    };
    
    spi_device_transmit(this -> spi_device, &eraseSectorTransaction);
  	heap_caps_free(cmd);
  	
    // Ждем завершения стирания
    uint8_t status;

    do
    {
        status = getMemoryStatus_1();
        esp_rom_delay_us(1);
    }
    while(status & 0x01);
}

uint8_t W25Q128 :: getMemoryStatus_1()
{
    // Но если хочешь transmit - выделяем DMA
    uint8_t* tx_buffer = (uint8_t*) heap_caps_aligned_alloc(32, 2, MALLOC_CAP_DMA);
    uint8_t* rx_buffer = (uint8_t*) heap_caps_aligned_alloc(32, 2, MALLOC_CAP_DMA);
    
    if (!tx_buffer || !rx_buffer)
    {
        if (tx_buffer) heap_caps_free(tx_buffer);
        if (rx_buffer) heap_caps_free(rx_buffer);
        return 0xFF; // Ошибка
    }
    
    tx_buffer[0] = 0x05;  // Read Status Register 1 command
    tx_buffer[1] = 0xFF;  // Dummy byte для тактирования ответа
    
    spi_transaction_t trans = {
        .flags = 0,
        .length = 16,      // 2 байта * 8 бит
        .rxlength = 16,
        .tx_buffer = tx_buffer,
        .rx_buffer = rx_buffer
    };
    
    esp_err_t ret = spi_device_transmit(this -> spi_device, &trans);
    
    uint8_t status = 0xFF;
    
    if (ret == ESP_OK)
    {
        status = rx_buffer[1]; // Ответ приходит во втором байте
    }
    
    heap_caps_free(tx_buffer);
    heap_caps_free(rx_buffer);
    
    return status;
}

uint8_t W25Q128 ::  getMemoryStatus_2()
{
    uint8_t* tx_buffer = (uint8_t*) heap_caps_aligned_alloc(32, 2, MALLOC_CAP_DMA);
    uint8_t* rx_buffer = (uint8_t*) heap_caps_aligned_alloc(32, 2, MALLOC_CAP_DMA);
    
    tx_buffer[0] = 0x35;
    tx_buffer[1] = 0xFF;
    
    spi_transaction_t trans = {
        .length = 16,
        .rxlength = 16,
        .tx_buffer = tx_buffer,
        .rx_buffer = rx_buffer
    };
    
    uint8_t status = 0xFF;
    esp_err_t err = spi_device_transmit(spi_device, &trans);
    
    if (err == ESP_OK)
    {
        status = rx_buffer[1]; // Ответ приходит во втором байте
    }
    
    return status;
}

void W25Q128 :: writeEnable()
{
 	// 1. Выделяем буфер в DMA-памяти (НЕ на стеке!)
    uint8_t* cmd = (uint8_t*) heap_caps_aligned_alloc(32, 1, MALLOC_CAP_DMA);
    
    if (cmd == NULL)
    {
        printf("ERROR: DMA malloc failed in writeEnable\n");
        return;
    }
    
    cmd[0] = 0x06; // WREN command
    
    spi_transaction_t trans =
    {
        .flags = 0,
        .length = 8,
        .rxlength = 0,
        .tx_buffer = cmd,
        .rx_buffer = NULL
    };
    
    // 2. Отправляем (Семафор уже захвачен вызывающей функцией)
    esp_err_t ret = spi_device_transmit(this -> spi_device, &trans);
    esp_rom_delay_us(50);
    
    // 3. Освобождаем память сразу после передачи
    heap_caps_free(cmd);
    
    if (ret != ESP_OK)
    {
        printf("ERROR: SPI writeEnable failed: %d\n", ret);
        return;
    }

    // 4. ВАЖНО: Проверяем, установился ли флаг WEL (бит 1)
    // Это гарантия того, что команда дошла до чипа
    uint8_t status = getMemoryStatus_1();
    if(!(status & 0x02))
    {
        printf("ERROR: WEL flag NOT set after writeEnable! Status: 0x%02X\n", status);
    }
}