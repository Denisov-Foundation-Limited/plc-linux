/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#include <stdio.h>

#include <glib-2.0/glib.h>
#include <jansson.h>
#include <fcgiapp.h>

#include <net/web/handlers/meteoh.h>
#include <net/web/response.h>
#include <utils/utils.h>
#include <utils/log.h>
#include <stack/rpc.h>

/*********************************************************************/
/*                                                                   */
/*                         PRIVATE FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

static bool HandlerSensorsGet(FCGX_Request *req, GList **params)
{
    json_t  *root = json_object();
    GList   *sensors = NULL;

    if (!RpcMeteoSensorsGet(RPC_DEFAULT_UNIT, &sensors)) {
        return ResponseFailSend(req, "METEOH", "Failed to get meteo sensors");
    }

    json_t *jsensors = json_array();

    for (GList *s = sensors; s != NULL; s = s->next) {
        RpcMeteoSensor *sensor = (RpcMeteoSensor *)s->data;

        json_t *jsensor = json_object();
        json_object_set_new(jsensor, "name", json_string(sensor->name));
        json_object_set_new(jsensor, "type", json_integer(sensor->type));
        switch (sensor->type) {
            case RPC_METEO_SENSOR_DS18B20:
                json_object_set_new(jsensor, "ds18b20", json_object());
                json_object_set_new(json_object_get(jsensor, "ds18b20"), "temp", json_real(sensor->ds18b20.temp));
                break;
        }
        json_array_append_new(jsensors, jsensor);

        free(sensor);
    }

    json_object_set_new(root, "sensors", jsensors);
    g_list_free(sensors);

    return ResponseOkSend(req, root);
}

/*********************************************************************/
/*                                                                   */
/*                          PUBLIC FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

bool HandlerMeteoProcess(FCGX_Request *req, GList **params)
{
    for (GList *p = *params; p != NULL; p = p->next) {
        UtilsReqParam *param = (UtilsReqParam *)p->data;

        if (!strcmp(param->name, "cmd")) {
            if (!strcmp(param->value, "sensors_get")) {
                return HandlerSensorsGet(req, params);
            } else {
                return false;
            }
        }
    }

    return true;
}
