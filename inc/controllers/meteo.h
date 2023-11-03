/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#ifndef __METEO_CTRL_H__
#define __METEO_CTRL_H__

#include <stdbool.h>

#include <glib-2.0/glib.h>

#include <utils/utils.h>

typedef enum {
    METEO_SENSOR_DS18B20
} MeteoSensorType;

typedef struct {
    char            name[SHORT_STR_LEN];
    MeteoSensorType type;
    char            id[SHORT_STR_LEN];
    float           temp;
    float           hum;
    float           pres;
    bool            error;
} MeteoSensor;

bool MeteoControllerStart();

GList **MeteoSensorsGet();

void MeteoSensorAdd(MeteoSensor *sensor);

#endif /* __METEO_CTRL_H__ */