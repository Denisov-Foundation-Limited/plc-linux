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
    size_t  ext_index;
    json_t  *ext_value;

    Log(LOG_TYPE_INFO, "CONFIGS", "Add Socket controller");

    json_array_foreach(json_object_get(data, "socket"), ext_index, ext_value) {
        json_t *jgpio = json_object_get(ext_value, "gpio");

        GpioPin *button = GpioPinGet(json_string_value(json_object_get(jgpio, "button")));
        if (button == NULL) {
            LogF(LOG_TYPE_ERROR, "CONFIGS", "Socket button gpio not found");
            return false;
        }

        GpioPin *relay = GpioPinGet(json_string_value(json_object_get(jgpio, "relay")));
        if (relay == NULL) {
            LogF(LOG_TYPE_ERROR, "CONFIGS", "Socket relay gpio not found");
            return false;
        }

         Socket *socket = SocketNew(
            json_string_value(json_object_get(ext_value, "name")),
            button,
            relay
        );

        SocketAdd(socket);

        LogF(LOG_TYPE_INFO, "CONFIGS", "Add Socket name: \"%s\"", socket->name);
    }

    return true;
}
