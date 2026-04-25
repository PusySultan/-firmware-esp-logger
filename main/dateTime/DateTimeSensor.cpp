/*
 * DateTimeSensor.cpp
 *
 *  Created on: 25 мар. 2026 г.
 *      Author: Kirill
 */

#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include "hal/gpio_types.h"
#include "DateTimeSensor.hpp"

static portMUX_TYPE my_spinlock = portMUX_INITIALIZER_UNLOCKED;

DateTimeSensor :: DateTimeSensor ()
{
  // TODO Auto-generated constructor stub

}

DateTimeSensor :: ~DateTimeSensor ()
{
  // TODO Auto-generated destructor stub
}

void DateTimeSensor:: ds1302_init(uint8_t ena, uint8_t clk, uint8_t dat)
{
	pin_ena = static_cast<gpio_num_t>(ena);
	pin_clk = static_cast<gpio_num_t>(clk);
	pin_dat = static_cast<gpio_num_t>(dat);
	
	// на выход пин enable
    // Настройка пина CE (выход)
    esp_rom_gpio_pad_select_gpio(pin_ena);
    gpio_set_direction(pin_ena, GPIO_MODE_OUTPUT);
    gpio_set_level(pin_ena, 0);   // CE = 0
	
    // Настройка пина CLK (выход)
    esp_rom_gpio_pad_select_gpio(pin_clk);
    gpio_set_direction(pin_clk, GPIO_MODE_OUTPUT);
    gpio_set_level(pin_clk, 0);   // CLK = 0
	
    // Настройка пина DAT (вход с подтяжкой)
    esp_rom_gpio_pad_select_gpio(pin_dat);
    gpio_set_direction(pin_dat, GPIO_MODE_INPUT);
    gpio_pullup_en(pin_dat);
    gpio_pulldown_dis(pin_dat);
}

void DateTimeSensor :: prepareRead(uint8_t address)
{
    gpio_set_direction(pin_dat, GPIO_MODE_OUTPUT);
    gpio_pullup_en(pin_dat);
    gpio_pulldown_dis(pin_dat);
    
    gpio_set_level(pin_ena, 1);
    
    uint8_t command = 0b10000001 | address;
    ds1302_write_byte(command);
    
    gpio_set_direction(pin_dat, GPIO_MODE_INPUT);
}

void DateTimeSensor:: prepareWrite(uint8_t address)
{
	gpio_set_direction(pin_dat, GPIO_MODE_OUTPUT);
    gpio_pullup_en(pin_dat);
    gpio_pulldown_dis(pin_dat);
	gpio_set_level(pin_ena, 1);
	
    uint8_t command = 0b10000000 | address;
    ds1302_write_byte(command);
}


void DateTimeSensor:: nextBit()
{
	gpio_set_level(pin_clk, 1);
	esp_rom_delay_us(1);
	
	gpio_set_level(pin_clk, 0);
	esp_rom_delay_us(1);
}

uint8_t DateTimeSensor::ds1302_readByte()
{
    uint8_t byte = 0;
    
    for (uint8_t b = 0; b < 8; b++)
    {
        if (gpio_get_level(pin_dat) == 1)
        {
			byte |= (0x01 << b);
		}
		
		nextBit();	
    }
    
    return byte;
}

void DateTimeSensor:: ds1302_write_byte(uint8_t data)
{	
	for(uint8_t b = 0; b < 8; b++)
    {
		gpio_set_level(pin_dat, (data & 0x01) ? 1 : 0);
        nextBit();
        data >>= 1;
    }
}

uint8_t DateTimeSensor:: ds1302_dec2bcd(uint8_t dec)
{
	return ((dec / 10 * 16) + (dec % 10));
}

uint8_t DateTimeSensor:: ds1302_bcd2dec(uint8_t bcd)
{
	return ((bcd / 16 * 10) + (bcd % 16));
}

void DateTimeSensor:: ds1302_getDateTime(DateTime *dt)
{
	taskENTER_CRITICAL(&my_spinlock);
		
    prepareRead(DS1302_REG_BURST);
 	
 	dt -> seconds = ds1302_bcd2dec(ds1302_readByte() & 0b01111111);
    dt -> minutes = ds1302_bcd2dec(ds1302_readByte( )& 0b01111111);
    dt -> hours   = ds1302_bcd2dec(ds1302_readByte() & 0b00111111);
    dt -> date    = ds1302_bcd2dec(ds1302_readByte() & 0b00111111);
    dt -> month   = ds1302_bcd2dec(ds1302_readByte() & 0b00011111);
    dt -> day     = ds1302_bcd2dec(ds1302_readByte() & 0b00000111);
    dt -> year    = ds1302_bcd2dec(ds1302_readByte() & 0b11111111);
    
    gpio_set_level(pin_ena, 0);          // Выключаем чип
    
	taskEXIT_CRITICAL(&my_spinlock);
}


void DateTimeSensor:: ds1302_setDateTime(DateTime *dt)
{
	taskENTER_CRITICAL(&my_spinlock);
	
	prepareWrite(DS1302_REG_WP);
	ds1302_write_byte(0b00000000);
	gpio_set_level(pin_ena, 0);

	prepareWrite(DS1302_REG_BURST);


    ds1302_write_byte(ds1302_dec2bcd(dt->seconds % 60));
    ds1302_write_byte(ds1302_dec2bcd(dt->minutes % 60));
    ds1302_write_byte(ds1302_dec2bcd(dt->hours   % 24));
    ds1302_write_byte(ds1302_dec2bcd(dt->date    % 32));
    ds1302_write_byte(ds1302_dec2bcd(dt->month   % 13));
    ds1302_write_byte(ds1302_dec2bcd(dt->day     % 8));
    ds1302_write_byte(ds1302_dec2bcd(dt->year    % 100));
    ds1302_write_byte(0b10000000);

    gpio_set_level(pin_ena, 0);
    
    taskEXIT_CRITICAL(&my_spinlock);
}