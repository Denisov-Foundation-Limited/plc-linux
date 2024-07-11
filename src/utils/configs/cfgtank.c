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

    if (data == NULL) {
        Log(LOG_TYPE_ERROR, "CONFIGS", "Tank data not found");
        return false;
    }

    Log(LOG_TYPE_INFO, "CONFIGS", "Add Tank controller");

    json_t *jtank = json_object_get(data, "tank");
    if (jtank == NULL) {
        Log(LOG_TYPE_ERROR, "CONFIGS", "Tank not found");
        return false;
    }

    json_array_foreach(jtank, ext_index, ext_value) {
        json_t *jname = json_object_get(ext_value, "name");
        if (jtank == NULL) {
            Log(LOG_TYPE_ERROR, "CONFIGS", "Tank name not found");
            return false;
        }

        Tank *tank = TankNew(json_string_value(jname));

        json_t *jgpio = json_object_get(ext_value, "gpio");
        if (jgpio == NULL) {
            Log(LOG_TYPE_ERROR, "CONFIGS", "Tank GPIO not found");
            return false;
        }

        json_t *jvalve = json_object_get(jgpio, "valve");
        if (jvalve == NULL) {
            Log(LOG_TYPE_ERROR, "CONFIGS", "Tank GPIO valve not found");
            return false;
        }

        GpioPin *valve = GpioPinGet(json_string_value(jvalve));
        if (valve == NULL) {
            LogF(LOG_TYPE_ERROR, "CONFIGS", "Tank GPIO valve not found in list");
            return false;
        }

        json_t *jpump = json_object_get(jgpio, "pump");
        if (jpump == NULL) {
            Log(LOG_TYPE_ERROR, "CONFIGS", "Tank GPIO pump not found");
            return false;
        }

        GpioPin *pump = GpioPinGet(json_string_value(jpump));
        if (pump == NULL) {
            LogF(LOG_TYPE_ERROR, "CONFIGS", "Tank GPIO pump not found in list");
            return false;
        }

        json_t *jempty = json_object_get(jgpio, "empty");
        if (jempty == NULL) {
            Log(LOG_TYPE_ERROR, "CONFIGS", "Tank GPIO empty not found");
            return false;
        }

        GpioPin *empty = GpioPinGet(json_string_value(jempty));
        if (empty == NULL) {
            LogF(LOG_TYPE_ERROR, "CONFIGS", "Tank GPIO empty not found in list");
            return false;
        }

        json_t *jfull = json_object_get(jgpio, "full");
        if (jfull == NULL) {
            Log(LOG_TYPE_ERROR, "CONFIGS", "Tank GPIO full not found");
            return false;
        }

        GpioPin *full = GpioPinGet(json_string_value(jfull));
        if (full == NULL) {
            LogF(LOG_TYPE_ERROR, "CONFIGS", "Tank GPIO full not found in list");
            return false;
        }

        json_t *jstatus = json_object_get(jgpio, "status");
        if (jstatus == NULL) {
            Log(LOG_TYPE_ERROR, "CONFIGS", "Tank GPIO status not found");
            return false;
        }

        json_t *jled = json_object_get(jstatus, "led");
        if (jled == NULL) {
            Log(LOG_TYPE_ERROR, "CONFIGS", "Tank GPIO status LED not found");
            return false;
        }

        GpioPin *led = GpioPinGet(json_string_value(jled));
        if (led == NULL) {
            LogF(LOG_TYPE_ERROR, "CONFIGS", "Tank GPIO status LED not found in list");
            return false;
        }

        json_t *jbutton = json_object_get(jstatus, "button");
        if (jbutton == NULL) {
            Log(LOG_TYPE_ERROR, "CONFIGS", "Tank GPIO status button not found");
            return false;
        }

        GpioPin *button = GpioPinGet(json_string_value(jbutton));
        if (button == NULL) {
            Log(LOG_TYPE_ERROR, "CONFIGS", "Tank GPIO status button not found in list");
            return false;
        }

        TankGpioSet(tank, TANK_GPIO_VALVE, valve);
        TankGpioSet(tank, TANK_GPIO_PUMP, pump);
        TankGpioSet(tank, TANK_GPIO_EMPTY, empty);
        TankGpioSet(tank, TANK_GPIO_FULL, full);
        TankGpioSet(tank, TANK_GPIO_STATUS_LED, led);
        TankGpioSet(tank, TANK_GPIO_STATUS_BUTTON, button);

        json_t *jlevels = json_object_get(ext_value, "levels");
        if (jlevels == NULL) {
            Log(LOG_TYPE_ERROR, "CONFIGS", "Tank levels not found");
            return false;
        }

        json_array_foreach(jlevels, index, value) {
            json_t *jgpio = json_object_get(value, "gpio");
            if (jgpio == NULL) {
                Log(LOG_TYPE_ERROR, "CONFIGS", "Tank levels GPIO not found");
                return false;
            }

            GpioPin *sensor = GpioPinGet(json_string_value(jgpio));
            if (sensor == NULL) {
                Log(LOG_TYPE_ERROR, "CONFIGS", "Tank levels GPIO not found in list");
                return false;
            }

            json_t *jper = json_object_get(value, "percent");
            if (jper == NULL) {
                Log(LOG_TYPE_ERROR, "CONFIGS", "Tank levels percent not found");
                return false;
            }

            json_t *jntf = json_object_get(value, "notify");
            if (jntf == NULL) {
                Log(LOG_TYPE_ERROR, "CONFIGS", "Tank levels notify not found");
                return false;
            }

            TankLevel *level = TankLevelNew(
                json_integer_value(jper),
                sensor,
                json_boolean_value(jntf)
            );

            TankLevelAdd(tank, level);
        }

        json_t *jstates = json_object_get(ext_value, "states");
        if (jstates == NULL) {
            Log(LOG_TYPE_ERROR, "CONFIGS", "Tank states not found");
            return false;
        }

        jvalve = json_object_get(jstates, "valve");
        if (jvalve == NULL) {
            Log(LOG_TYPE_ERROR, "CONFIGS", "Tank states valve not found");
            return false;
        }

        json_t *jon = json_object_get(jvalve, "on");
        if (jon == NULL) {
            Log(LOG_TYPE_ERROR, "CONFIGS", "Tank states valve on not found");
            return false;
        }

        json_t *joff = json_object_get(jvalve, "off");
        if (joff == NULL) {
            Log(LOG_TYPE_ERROR, "CONFIGS", "Tank states valve off not found");
            return false;
        }

        TankState *valve_state = TankStateNew(
                json_integer_value(jon),
                json_integer_value(joff)
            );

        jpump = json_object_get(jstates, "pump");
        if (jpump == NULL) {
            Log(LOG_TYPE_ERROR, "CONFIGS", "Tank states pump not found");
            return false;
        }

        jon = json_object_get(jpump, "on");
        if (jon == NULL) {
            Log(LOG_TYPE_ERROR, "CONFIGS", "Tank states pump on not found");
            return false;
        }

        joff = json_object_get(jpump, "off");
        if (joff == NULL) {
            Log(LOG_TYPE_ERROR, "CONFIGS", "Tank states pump off not found");
            return false;
        }

        TankState *pump_state = TankStateNew(
                json_integer_value(jon),
                json_integer_value(joff)
            );

        TankStateSet(tank, TANK_STATE_PUMP, pump_state);
        TankStateSet(tank, TANK_STATE_VALVE, valve_state);

        TankAdd(tank);

        LogF(LOG_TYPE_INFO, "CONFIGS", "Add Tank name: \"%s\"", tank->name);
    }

    return true;
}
