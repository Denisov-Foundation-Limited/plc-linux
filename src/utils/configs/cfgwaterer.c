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
#include <controllers/tank.h>
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

    if (data == NULL) {
        Log(LOG_TYPE_ERROR, "CONFIGS", "Configs data not found");
        return false;
    }

    Log(LOG_TYPE_INFO, "CONFIGS", "Add Waterer controller");

    json_t *jwtr = json_object_get(data, "waterer");
    if (jwtr == NULL) {
        Log(LOG_TYPE_WARN, "CONFIGS", "Waterer data not found");
        return true;
    }

    json_array_foreach(jwtr, ext_index, ext_value) {
        json_t *jtnk = json_object_get(ext_value, "tank");
        if (jtnk == NULL) {
            Log(LOG_TYPE_ERROR, "CONFIGS", "Waterer not found");
            return false;
        }

        Tank *tank = TankGet(json_string_value(jtnk));
        if (tank == NULL) {
            Log(LOG_TYPE_ERROR, "CONFIGS", "Waterer tank not found");
            return false;
        }

        json_t *jname = json_object_get(ext_value, "name");
        if (jname == NULL) {
            Log(LOG_TYPE_ERROR, "CONFIGS", "Waterer name not found");
            return false;
        }

        Waterer *wtr = WatererNew(json_string_value(jname), tank);

        json_t *jgpio = json_object_get(ext_value, "gpio");
        if (jgpio == NULL) {
            Log(LOG_TYPE_ERROR, "CONFIGS", "Waterer GPIO not found");
            return false;
        }

        json_t *jvalve = json_object_get(jgpio, "valve");
        if (jgpio == NULL) {
            Log(LOG_TYPE_ERROR, "CONFIGS", "Waterer GPIO valve not found");
            return false;
        }

        GpioPin *valve = GpioPinGet(json_string_value(jvalve));
        if (valve == NULL) {
            LogF(LOG_TYPE_ERROR, "CONFIGS", "Waterer GPIO valve not found in list");
            return false;
        }

        json_t *jstatus = json_object_get(jgpio, "status");
        if (jstatus == NULL) {
            LogF(LOG_TYPE_ERROR, "CONFIGS", "Waterer GPIO status not found");
            return false;
        }

        json_t *jled = json_object_get(jstatus, "led");
        if (jgpio == NULL) {
            Log(LOG_TYPE_ERROR, "CONFIGS", "Waterer GPIO not found");
            return false;
        }

        json_t *jbtn = json_object_get(jstatus, "button");
        if (jbtn == NULL) {
            Log(LOG_TYPE_ERROR, "CONFIGS", "Waterer GPIO not found");
            return false;
        }

        GpioPin *led = GpioPinGet(json_string_value(jled));
        if (led == NULL) {
            LogF(LOG_TYPE_ERROR, "CONFIGS", "Waterer GPIO led not found in list");
            return false;
        }

        GpioPin *button = GpioPinGet(json_string_value(jbtn));
        if (button == NULL) {
            Log(LOG_TYPE_ERROR, "CONFIGS", "Waterer GPIO button not found in list");
            return false;
        }

        WatererGpioSet(wtr, WATERER_GPIO_VALVE, valve);
        WatererGpioSet(wtr, WATERER_GPIO_STATUS_LED, led);
        WatererGpioSet(wtr, WATERER_GPIO_STATUS_BUTTON, button);

        json_t *jtimes = json_object_get(ext_value, "times");
        if (jtimes == NULL) {
            Log(LOG_TYPE_ERROR, "CONFIGS", "Waterer times not found");
            return false;
        }

        json_array_foreach(jtimes, index, value) {
            PlcTime t;

            json_t *jday = json_object_get(value, "day");
            if (jday == NULL) {
                Log(LOG_TYPE_ERROR, "CONFIGS", "Waterer times day not found");
                return false;
            }

            json_t *jmin = json_object_get(value, "min");
            if (jmin == NULL) {
                Log(LOG_TYPE_ERROR, "CONFIGS", "Waterer times min not found");
                return false;
            }

            json_t *jhour = json_object_get(value, "hour");
            if (jhour == NULL) {
                Log(LOG_TYPE_ERROR, "CONFIGS", "Waterer times hour not found");
                return false;
            }

            json_t *jstate = json_object_get(value, "state");
            if (jstate == NULL) {
                Log(LOG_TYPE_ERROR, "CONFIGS", "Waterer times state not found");
                return false;
            }

            json_t *jnotify = json_object_get(value, "notify");
            if (jnotify == NULL) {
                Log(LOG_TYPE_ERROR, "CONFIGS", "Waterer times notify not found");
                return false;
            }

            t.dow = json_integer_value(jday);
            t.min = json_integer_value(jmin);
            t.hour = json_integer_value(jhour);

            WateringTime *time = WateringTimeNew(
                t,
                json_boolean_value(jstate),
                json_boolean_value(jnotify)
            );

            WatererTimeAdd(wtr, time);
        }

        WatererAdd(wtr);

        LogF(LOG_TYPE_INFO, "CONFIGS", "Add Waterer name: \"%s\"", wtr->name);
    }

    return true;
}
