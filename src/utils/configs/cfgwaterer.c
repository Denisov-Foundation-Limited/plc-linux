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
#include <controllers/waterer.h>
#include <core/gpio.h>
#include <utils/configs/cfgwaterer.h>

/*********************************************************************/
/*                                                                   */
/*                          PUBLIC FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

bool CfgWatererLoad(json_t *data)
{
    size_t  ext_index, index;
    json_t  *ext_value, *value;

    Log(LOG_TYPE_INFO, "CONFIGS", "Add Waterer controller");

    json_array_foreach(json_object_get(data, "waterer"), ext_index, ext_value) {
        Waterer *wtr = WatererNew(json_string_value(json_object_get(ext_value, "name")));

        json_t *jgpio = json_object_get(ext_value, "gpio");

        GpioPin *valve = GpioPinGet(json_string_value(json_object_get(jgpio, "valve")));
        if (valve == NULL) {
            LogF(LOG_TYPE_ERROR, "CONFIGS", "Waterer valve gpio not found");
            return false;
        }

        json_t *jstatus = json_object_get(jgpio, "status");

        GpioPin *led = GpioPinGet(json_string_value(json_object_get(jstatus, "led")));
        if (led == NULL) {
            LogF(LOG_TYPE_ERROR, "CONFIGS", "Waterer led gpio not found");
            return false;
        }

        GpioPin *button = GpioPinGet(json_string_value(json_object_get(jstatus, "button")));
        if (button == NULL) {
            Log(LOG_TYPE_ERROR, "CONFIGS", "Waterer button gpio not found");
            return false;
        }

        WatererGpioSet(wtr, WATERER_GPIO_VALVE, valve);
        WatererGpioSet(wtr, WATERER_GPIO_STATUS_LED, led);
        WatererGpioSet(wtr, WATERER_GPIO_STATUS_BUTTON, button);

        json_array_foreach(json_object_get(ext_value, "times"), index, value) {
            PlcTime t;

            t.dow = json_integer_value(json_object_get(value, "day"));
            t.min = json_integer_value(json_object_get(value, "min"));
            t.hour = json_integer_value(json_object_get(value, "hour"));

            WateringTime *time = WateringTimeNew(
                t,
                json_boolean_value(json_object_get(value, "state")),
                json_boolean_value(json_object_get(value, "notify"))
            );

            WatererTimeAdd(wtr, time);
        }

        WatererAdd(wtr);

        LogF(LOG_TYPE_INFO, "CONFIGS", "Add Waterer name: \"%s\"", wtr->name);
    }

    return true;
}
