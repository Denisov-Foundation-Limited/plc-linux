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
#include <controllers/tank.h>
#include <core/gpio.h>
#include <utils/configs/cfgtank.h>

/*********************************************************************/
/*                                                                   */
/*                          PUBLIC FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

bool CfgTankLoad(json_t *data)
{
    size_t  ext_index, index;
    json_t  *ext_value, *value;

    Log(LOG_TYPE_INFO, "CONFIGS", "Add Tank controller");

    json_array_foreach(json_object_get(data, "tank"), ext_index, ext_value) {
        Tank *tank = TankNew(json_string_value(json_object_get(ext_value, "name")));

        json_t *jgpio = json_object_get(ext_value, "gpio");

        GpioPin *valve = GpioPinGet(json_string_value(json_object_get(jgpio, "valve")));
        if (valve == NULL) {
            LogF(LOG_TYPE_ERROR, "CONFIGS", "Tank valve gpio not found");
            return false;
        }

        GpioPin *pump = GpioPinGet(json_string_value(json_object_get(jgpio, "pump")));
        if (pump == NULL) {
            LogF(LOG_TYPE_ERROR, "CONFIGS", "Tank pump gpio not found");
            return false;
        }

        json_t *jstatus = json_object_get(jgpio, "status");

        GpioPin *led = GpioPinGet(json_string_value(json_object_get(jstatus, "led")));
        if (led == NULL) {
            LogF(LOG_TYPE_ERROR, "CONFIGS", "Tank led gpio not found");
            return false;
        }

        GpioPin *button = GpioPinGet(json_string_value(json_object_get(jstatus, "button")));
        if (button == NULL) {
            Log(LOG_TYPE_ERROR, "CONFIGS", "Tank button gpio not found");
            return false;
        }

        TankGpioSet(tank, TANK_GPIO_VALVE, valve);
        TankGpioSet(tank, TANK_GPIO_PUMP, pump);
        TankGpioSet(tank, TANK_GPIO_STATUS_LED, led);
        TankGpioSet(tank, TANK_GPIO_STATUS_BUTTON, button);

        json_array_foreach(json_object_get(ext_value, "levels"), index, value) {
            GpioPin *sensor = GpioPinGet(json_string_value(json_object_get(value, "gpio")));
            if (sensor == NULL) {
                Log(LOG_TYPE_ERROR, "CONFIGS", "Tank level gpio not found");
                return false;
            }

            TankLevel *level = TankLevelNew(
                json_integer_value(json_object_get(value, "percent")),
                sensor,
                json_boolean_value(json_object_get(value, "notify"))
            );

            TankLevelAdd(tank, level);
        }

        json_t *jstates = json_object_get(ext_value, "states");
        json_t *jvalve = json_object_get(jstates, "valve");
        json_t *jpump = json_object_get(jstates, "pump");

        TankState *valve_state = TankStateNew(
                json_integer_value(json_object_get(jvalve, "on")),
                json_integer_value(json_object_get(jvalve, "off"))
            );

        TankState *pump_state = TankStateNew(
                json_integer_value(json_object_get(jpump, "on")),
                json_integer_value(json_object_get(jpump, "off"))
            );

        TankStateSet(tank, TANK_STATE_PUMP, pump_state);
        TankStateSet(tank, TANK_STATE_VALVE, valve_state);

        TankAdd(tank);

        LogF(LOG_TYPE_INFO, "CONFIGS", "Add Tank name: \"%s\"", tank->name);
    }

    return true;
}
