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

#define METEO_SENSOR_TRIES  5

typedef enum {
    METEO_SENSOR_DS18B20
} MeteoSensorType;

typedef struct {
    char    id[SHORT_STR_LEN];
    float   temp;
} MeteoDs18b20;

typedef struct {
    char            name[SHORT_STR_LEN];
    MeteoSensorType type;
    MeteoDs18b20    ds18b20;
    bool            error;
} MeteoSensor;

/**
 * @brief Alloc mem for new sensor
 * 
 * @param name Name of sensor
 * @param type Type of sensor
 * 
 * @return Meteo sensor
 */
MeteoSensor *MeteoSensorNew(const char *name, MeteoSensorType type);

/**
 * @brief Starting meteo controller
 *
 * @return Result of starting
 */
bool MeteoControllerStart();

/**
 * @brief Get all meteo sensors list
 * 
 * @return Meteo sensors 
 */
GList **MeteoSensorsGet();

/**
 * @brief Get meteo sensor by name
 * 
 * @param name Meteo sensor name
 *
 * @return Meteo sensor object
 */
MeteoSensor *MeteoSensorGet(const char *name);

/**
 * @brief Add new meteo sensor
 * 
 * @param sensor New meteo sensor struct
 */
void MeteoSensorAdd(MeteoSensor *sensor);

#endif /* __METEO_CTRL_H__ */
