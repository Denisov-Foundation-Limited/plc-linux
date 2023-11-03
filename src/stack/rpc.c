/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#include <stack/rpc.h>
#include <controllers/security.h>
#include <utils/log.h>
#include <controllers/meteo.h>
#include <controllers/socket.h>

#include <stdlib.h>

bool RpcSecurityStatusSet(unsigned unit, bool status)
{
    if (unit == 0) {
        return SecurityStatusSet(status, true);
    }

    return true;
}

bool RpcSecurityStatusGet(unsigned unit, bool *status)
{
    if (status == NULL) {
        return false;
    }

    if (unit == 0) {
        *status = SecurityStatusGet();
        return true;
    }

    return true;
}

bool RpcSecurityAlarmSet(unsigned unit, bool alarm)
{
    if (unit == 0) {
        return SecurityAlarmSet(alarm, false);
    }

    return true;
}

bool RpcSecurityAlarmGet(unsigned unit, bool *alarm)
{
    if (alarm == NULL) {
        return false;
    }

    if (unit == 0) {
        *alarm = SecurityAlarmGet();
        return true;
    }

    return true;
}

bool RpcSecuritySensorsGet(unsigned unit, GList **sensors)
{
    if (sensors == NULL) {
        return false;
    }

    if (unit == 0) {
        for (GList *c = *SecuritySensorsGet(); c != NULL; c = c->next) {
            SecuritySensor *sensor = (SecuritySensor *)c->data;

            RpcSecuritySensor *s = (RpcSecuritySensor *)malloc(sizeof(RpcSecuritySensor));
            strncpy(s->name, sensor->name, SHORT_STR_LEN);
            s->detected = sensor->detected;
            s->type = (unsigned)sensor->type;

            *sensors = g_list_append(*sensors, s);
        }
        return true;
    }

    return true;
}

bool RpcMeteoSensorsGet(unsigned unit, GList **sensors)
{
    if (sensors == NULL) {
        return false;
    }

    if (unit == 0) {
        for (GList *c = *MeteoSensorsGet(); c != NULL; c = c->next) {
            MeteoSensor *sensor = (MeteoSensor *)c->data;

            RpcMeteoSensor *s =(RpcMeteoSensor *)malloc(sizeof(RpcMeteoSensor));
            strncpy(s->name, sensor->name, SHORT_STR_LEN);
            s->temp = sensor->temp;
            s->hum = sensor->hum;
            s->pres = sensor->pres;

            *sensors = g_list_append(*sensors, (void *)s);
        }
        return true;
    }

    return true;
}

bool RpcSocketStatusSet(unsigned unit, const char *name, bool status)
{
    if (unit == 0) {
        Socket *socket = SocketGet(name);
        if (socket == NULL) {
            return false;
        }
        return SocketStatusSet(socket, status, true);
    }
    return true;
}

bool RpcSocketStatusGet(unsigned unit, const char *name)
{
    if (unit == 0) {
        Socket *socket = SocketGet(name);
        if (socket == NULL) {
            return false;
        }
        return SocketStatusGet(socket);
    }
    return true;
}

bool RpcSocketsGet(unsigned unit, GList **sockets)
{
    if (sockets == NULL) {
        return false;
    }

    if (unit == 0) {
        for (GList *c = *SocketsGet(); c != NULL; c = c->next) {
            Socket *socket = (Socket *)c->data;

            RpcSocket *s = (RpcSocket *)malloc(sizeof(RpcSocket));
            strncpy(s->name, socket->name, SHORT_STR_LEN);
            s->status = socket->status;

            *sockets = g_list_append(*sockets, (void *)s);
        }
        return true;
    }

    return true;
}
