/*
 * ISensor.cpp
 *
 *  Created on: 2 мар. 2026 г.
 *      Author: Kirill
 */

#include "ISensor.hpp"
#include "addresFunc.hpp"
#include "memory_w25q_const.hpp"

ISensor :: ISensor (SensorsID id)
{
	this -> ID = id;
}

ISensor :: ~ISensor ()
{
  // TODO Auto-generated destructor stub
}

SensorsID ISensor :: getSensorID()
{
	return this -> ID;
}

uint32_t ISensor :: getPointer()
{
	if(this -> curent_pointer != UINT32_MAX)
	{
		uint32_t temp = this -> curent_pointer;
		calculatePointer();
		
		return temp;
	}
	
	esp_err_t err = nvs_open("SETTINGS", NVS_READWRITE, &this -> nvs_handle);
    
    if (err != ESP_OK)
    {
        printf("NVS Open Error for sensor %d: %s\n", this -> ID, esp_err_to_name(err));
        return sector_addr_t[this -> ID].first; 
    }
	
 	// 2. Формируем ключ (например, "k1" для ID=1)
    char save_name[4]; 
    snprintf(save_name, sizeof(save_name), "k%d", this -> ID);
		
	 // 3. Пытаемся прочитать сохраненный указатель
    err = nvs_get_u32(this -> nvs_handle, save_name, &this -> curent_pointer);	
		
	if (err != ESP_OK)
	{
        // Ключ не найден (первый запуск) -> инициализируем началом сектора
        printf("Key %s not found. Init pointer to sector start\n", save_name);
        this -> curent_pointer = sector_addr_t[this -> ID].first;
        
        // ВАЖНО: Сразу сохраняем инициализированное значение!
        savePointer(); 
    }
	
	// Возвращаем текущий указатель и сдвигаем его для следующего вызова
    uint32_t result = this -> curent_pointer;
    calculatePointer();
    return result;
}

void ISensor :: savePointer()
{
	 if (this->nvs_handle == 0)
	 {
        // Если NVS не был открыт, пробуем открыть
        if (nvs_open("SETTING_NAMESPACE", NVS_READWRITE, &this->nvs_handle) != ESP_OK)
        {
            printf("NVS Open Error in savePointer (sensor %d)\n", this->ID);
            return;
        }
    }
    
	char save_name[4];
    snprintf(save_name, sizeof(save_name), "k%d", this->ID);
	
	 esp_err_t err = nvs_set_u32(this->nvs_handle, save_name, this->curent_pointer);
    
    if (err == ESP_OK) {
        err = nvs_commit(this -> nvs_handle);
    }
    
    if (err != ESP_OK) {
        printf("NVS Save Error for sensor %d: %s\n", this->ID, esp_err_to_name(err));
    }
}

void ISensor :: calculatePointer()
{	
	this -> curent_pointer += NEXT_SECTOR;
	
	if(this -> curent_pointer > sector_addr_t[ID].second)
	{
		curent_pointer = sector_addr_t[ID].first;
	}	
}

