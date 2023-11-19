/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#include <utils/log.h>
#include <controllers/meteo.h>
#include <core/gpio.h>
#include <utils/configs/cfgmeteo.h>

/*********************************************************************/
/*                                                                   */
/*                         PRIVATE FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

static bool CfgMeteoSensorsLoad(json_t *jmeteo)
{
    size_t  ext_index;
    json_t  *ext_value;

    json_array_foreach(json_object_get(jmeteo, "sensors"), ext_index, ext_value) {
        MeteoSensor *sensor;

        if (!strcmp(json_string_value(json_object_get(ext_value, "type")), "ds18b20")) {
            sensor = MeteoSensorNew(
                json_string_value(json_object_get(ext_value, "name")),
                METEO_SENSOR_DS18B20
            );
            strncpy(sensor->ds18b20.id, json_string_value(json_object_get(ext_value, "id")), SHORT_STR_LEN);
        } else {
            Log(LOG_TYPE_INFO, "CONFIGS", "Invalid meteo sensor type");
            return false;
        }

        MeteoSensorAdd(sensor);

        LogF(LOG_TYPE_INFO, "CONFIGS", "Add Meteo sensor name: \"%s\" type: \"%s\"",
            sensor->name, json_string_value(json_object_get(ext_value, "type")));
    }

    return true;
}

/*********************************************************************/
/*                                                                   */
/*                          PUBLIC FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

bool CfgMeteoLoad(json_t *data)
{
    json_t *jmeteo = json_object_get(data, "meteo");
    if (jmeteo == NULL) {
        return false;
    }

    Log(LOG_TYPE_INFO, "CONFIGS", "Add Meteo controller");

    if (!CfgMeteoSensorsLoad(jmeteo)) {
        Log(LOG_TYPE_ERROR, "CONFIGS", "Failed to load meteo sensors configs");
        return false;
    }

    return true;
}
