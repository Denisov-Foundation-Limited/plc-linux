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

#include <glib.h>

#include <utils/utils.h>

#define ONE_WIRE_PATH               "/sys/bus/w1/devices"

#define ONE_WIRE_DS18B20_PREFIX     "28"
#define ONE_WIRE_IBUTTON_PREFIX     "01"

#define ONE_WIRE_INVALID_TEMP       -127

typedef struct {
    char value[STR_LEN];
} OneWireData;

bool OneWireKeysRead(GList **keys);

bool OneWireTempRead(const char *id, float *temp);

#endif /* __ONE_WIRE_H__ */
