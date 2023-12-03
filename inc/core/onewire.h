/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#ifndef __ONE_WIRE_H__
#define __ONE_WIRE_H__

#include <stdbool.h>

#include <glib-2.0/glib.h>

#include <utils/utils.h>

#define ONE_WIRE_PATH               "/sys/bus/w1/devices"
#define ONE_WIRE_SLAVES_PATH        "/sys/bus/w1/drivers/w1_master_driver/w1_bus_master1/w1_master_slaves"

#define ONE_WIRE_DS18B20_PREFIX     "28"
#define ONE_WIRE_IBUTTON_PREFIX     "01"

#define ONE_WIRE_INVALID_TEMP       -127

typedef struct {
    char    value[SHORT_STR_LEN];
} OneWireData;

/**
 * @brief Read all detected 1-Wire bus devices
 * 
 * @param keys Readed devices list
 * 
 * @return true/false as result of reading devices
 */
bool OneWireDevicesList(GList **devices);

/**
 * @brief Read all detected iButton keys on 1-Wire bus
 * 
 * @param keys Readed keys list
 * 
 * @return true/false as result of reading keys
 */
bool OneWireKeysRead(GList **keys);

/**
 * @brief Read DS18B20 sensor temperature by ID on 1-Wire bus
 * 
 * @param id DS18B20 sensor id
 * @param temp Output temperature
 * 
 * @return true/false as result of reading temperature
 */
bool OneWireTempRead(const char *id, float *temp);

#endif /* __ONE_WIRE_H__ */
