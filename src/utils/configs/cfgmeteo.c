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

    if (jmeteo == NULL) {
        Log(LOG_TYPE_ERROR, "CONFIGS", "Meteo not found");
        return false;
    }

    json_t *jsensors = json_object_get(jmeteo, "sensors");
    if (jsensors == NULL) {
        Log(LOG_TYPE_ERROR, "CONFIGS", "Meteo sensors not found");
        return false;
    }

    json_array_foreach(jsensors, ext_index, ext_value) {
        MeteoSensor *sensor;

        json_t *jtype = json_object_get(ext_value, "type");
        if (jtype == NULL) {
            Log(LOG_TYPE_ERROR, "CONFIGS", "Meteo sensors type not found");
            return false;
        }

        json_t *jname = json_object_get(ext_value, "name");
        if (jname == NULL) {
            Log(LOG_TYPE_ERROR, "CONFIGS", "Meteo sensors name not found");
            return false;
        }

        if (!strcmp(json_string_value(jtype), "ds18b20")) {
            sensor = MeteoSensorNew(
                json_string_value(jname),
                METEO_SENSOR_DS18B20
            );

            json_t *jid = json_object_get(ext_value, "id");
            if (jid == NULL) {
                Log(LOG_TYPE_ERROR, "CONFIGS", "Meteo sensors name not found");
                return false;
            }

            strncpy(sensor->ds18b20.id, json_string_value(jid), SHORT_STR_LEN);
        } else {
            Log(LOG_TYPE_INFO, "CONFIGS", "Invalid meteo sensor type");
            return false;
        }

        MeteoSensorAdd(sensor);

        LogF(LOG_TYPE_INFO, "CONFIGS", "Add Meteo sensor name: \"%s\" type: \"%s\"",
            sensor->name, json_string_value(jtype));
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
    if (data == NULL) {
        Log(LOG_TYPE_ERROR, "CONFIGS", "Meteo data found");
        return false;
    }

    json_t *jmeteo = json_object_get(data, "meteo");
    if (jmeteo == NULL) {
        Log(LOG_TYPE_WARN, "CONFIGS", "Meteo data not found");
        return true;
    }

    Log(LOG_TYPE_INFO, "CONFIGS", "Add Meteo controller");

    if (!CfgMeteoSensorsLoad(jmeteo)) {
        Log(LOG_TYPE_ERROR, "CONFIGS", "Failed to load meteo sensors configs");
        return false;
    }

    return true;
}
