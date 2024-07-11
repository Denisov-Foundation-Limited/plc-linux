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
#include <controllers/socket.h>
#include <core/gpio.h>
#include <utils/configs/cfgsocket.h>

/*********************************************************************/
/*                                                                   */
/*                          PUBLIC FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

bool CfgSocketLoad(json_t *data)
{
    size_t      ext_index;
    json_t      *ext_value;
    SocketGroup grp;

    if (data == NULL) {
        Log(LOG_TYPE_ERROR, "CONFIGS", "Socket data not found");
        return false;
    }

    Log(LOG_TYPE_INFO, "CONFIGS", "Add Socket controller");

    json_t *jsocket = json_object_get(data, "socket");
    if (jsocket == NULL) {
        Log(LOG_TYPE_ERROR, "CONFIGS", "Socket not found");
        return false;
    }

    json_array_foreach(jsocket, ext_index, ext_value) {
        json_t *jgpio = json_object_get(ext_value, "gpio");
        if (jgpio == NULL) {
            Log(LOG_TYPE_ERROR, "CONFIGS", "Socket GPIO not found");
            return false;
        }

        json_t *jbutton = json_object_get(jgpio, "button");
        if (jbutton == NULL) {
            Log(LOG_TYPE_ERROR, "CONFIGS", "Socket GPIO button not found");
            return false;
        }

        GpioPin *button = GpioPinGet(json_string_value(jbutton));
        if (button == NULL) {
            LogF(LOG_TYPE_ERROR, "CONFIGS", "Socket GPIO button not found in list");
            return false;
        }

        json_t *jrel = json_object_get(jgpio, "relay");
        if (jrel == NULL) {
            Log(LOG_TYPE_ERROR, "CONFIGS", "Socket GPIO relay not found");
            return false;
        }

        GpioPin *relay = GpioPinGet(json_string_value(jrel));
        if (relay == NULL) {
            LogF(LOG_TYPE_ERROR, "CONFIGS", "Socket GPIO relay not found in list");
            return false;
        }

        json_t *jgrp = json_object_get(ext_value, "group");
        if (jgrp == NULL) {
            Log(LOG_TYPE_ERROR, "CONFIGS", "Socket group not found");
            return false;
        }

        if (!strcmp(json_string_value(jgrp), "light")) {
            grp = SOCKET_GROUP_LIGHT;
        } else if (!strcmp(json_string_value(jgrp), "socket")) {
            grp = SOCKET_GROUP_SOCKET;
        } else {
            Log(LOG_TYPE_ERROR, "CONFIGS", "Socket group unknown");
            return false;
        }

        json_t *jname = json_object_get(ext_value, "name");
        if (jname == NULL) {
            Log(LOG_TYPE_ERROR, "CONFIGS", "Socket name not found");
            return false;
        }

         Socket *socket = SocketNew(
            json_string_value(jname),
            button,
            relay,
            grp
        );

        SocketAdd(socket);

        LogF(LOG_TYPE_INFO, "CONFIGS", "Add Socket name: \"%s\"", socket->name);
    }

    return true;
}
