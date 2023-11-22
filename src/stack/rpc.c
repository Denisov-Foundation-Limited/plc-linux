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
#include <controllers/tank.h>
#include <utils/log.h>
#include <controllers/meteo.h>
#include <controllers/socket.h>
#include <net/web/webclient.h>
#include <net/web/webserver.h>
#include <utils/utils.h>
#include <stack/stack.h>

#include <stdlib.h>

#include <jansson.h>

/*********************************************************************/
/*                                                                   */
/*                          PUBLIC FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

/*********************************************************************/
/*                                                                   */
/*                           STACK MANAGMENT                         */
/*                                                                   */
/*********************************************************************/

bool RpcUnitStatusCheck(unsigned unit)
{
    char            buf[BUFFER_LEN_MAX];
    char            url[STR_LEN];
    json_error_t    error;

    if (unit == RPC_DEFAULT_UNIT) {
        return true;
    }

    StackUnit *u = StackUnitGet(unit);
    if (u == NULL) {
        return false;
    }

    snprintf(url, STR_LEN, "http://%s:%d/", u->ip, u->port);
    memset(buf, 0x0, BUFFER_LEN_MAX);

    if (!WebClientRequest(WEB_REQ_GET, url, NULL, buf)) {
        return false;
    }

    json_t *root = json_loads(buf, 0, &error);
    if (root == NULL) {
        return false;
    }

    if (!json_boolean_value(json_object_get(root, "result"))) {
        json_decref(root);
        return false;
    }

    json_decref(root);
    return true;
}

/*********************************************************************/
/*                                                                   */
/*                         SECURITY FUNCTIONS                        */
/*                                                                   */
/*********************************************************************/

bool RpcSecurityStatusSet(unsigned unit, bool status)
{
    char            buf[BUFFER_LEN_MAX];
    char            url[STR_LEN];
    json_error_t    error;

    if (unit == RPC_DEFAULT_UNIT) {
        return SecurityStatusSet(status, true);
    }

    StackUnit *u = StackUnitGet(unit);
    if (u == NULL) {
        return false;
    }

    snprintf(url, STR_LEN, "http://%s:%d/api/%s/security?cmd=status_set&status=%s",
            u->ip, u->port, SERVER_API_VER, (status == true) ? "true" : "false");
    memset(buf, 0x0, BUFFER_LEN_MAX);

    if (!WebClientRequest(WEB_REQ_GET, url, NULL, buf)) {
        return false;
    }

    json_t *root = json_loads(buf, 0, &error);
    if (root == NULL) {
        return false;
    }

    if (!json_boolean_value(json_object_get(root, "result"))) {
        json_decref(root);
        return false;
    }

    json_decref(root);
    return true;
}

bool RpcSecurityStatusGet(unsigned unit, bool *status)
{
    char            buf[BUFFER_LEN_MAX];
    char            url[STR_LEN];
    json_error_t    error;

    if (status == NULL) {
        return false;
    }

    if (unit == RPC_DEFAULT_UNIT) {
        *status = SecurityStatusGet();
        return true;
    }

    StackUnit *u = StackUnitGet(unit);
    if (u == NULL) {
        return false;
    }

    snprintf(url, STR_LEN, "http://%s:%d/api/%s/security?cmd=status_get", u->ip, u->port, SERVER_API_VER);
    memset(buf, 0x0, BUFFER_LEN_MAX);

    if (!WebClientRequest(WEB_REQ_GET, url, NULL, buf)) {
        return false;
    }

    json_t *root = json_loads(buf, 0, &error);
    if (root == NULL) {
        return false;
    }

    if (!json_boolean_value(json_object_get(root, "result"))) {
        json_decref(root);
        return false;
    }

    *status = json_boolean_value(json_object_get(root, "status"));

    json_decref(root);
    return true;
}

bool RpcSecurityAlarmSet(unsigned unit, bool alarm)
{
    char            buf[BUFFER_LEN_MAX];
    char            url[STR_LEN];
    json_error_t    error;

    if (unit == RPC_DEFAULT_UNIT) {
        return SecurityAlarmSet(alarm, false);
    }

    StackUnit *u = StackUnitGet(unit);
    if (u == NULL) {
        return false;
    }

    snprintf(url, STR_LEN, "http://%s:%d/api/%s/security?cmd=alarm_set&alarm=%s",
            u->ip, u->port, SERVER_API_VER, (alarm == true) ? "true" : "false");
    memset(buf, 0x0, BUFFER_LEN_MAX);

    if (!WebClientRequest(WEB_REQ_GET, url, NULL, buf)) {
        return false;
    }

    json_t *root = json_loads(buf, 0, &error);
    if (root == NULL) {
        return false;
    }

    if (!json_boolean_value(json_object_get(root, "result"))) {
        json_decref(root);
        return false;
    }

    json_decref(root);
    return true;
}

bool RpcSecurityAlarmGet(unsigned unit, bool *alarm)
{
    char            buf[BUFFER_LEN_MAX];
    char            url[STR_LEN];
    json_error_t    error;

    if (alarm == NULL) {
        return false;
    }

    if (unit == RPC_DEFAULT_UNIT) {
        *alarm = SecurityAlarmGet();
        return true;
    }

    StackUnit *u = StackUnitGet(unit);
    if (u == NULL) {
        return false;
    }

    snprintf(url, STR_LEN, "http://%s:%d/api/%s/security?cmd=alarm_get", u->ip, u->port, SERVER_API_VER);
    memset(buf, 0x0, BUFFER_LEN_MAX);

    if (!WebClientRequest(WEB_REQ_GET, url, NULL, buf)) {
        return false;
    }

    json_t *root = json_loads(buf, 0, &error);
    if (root == NULL) {
        return false;
    }

    if (!json_boolean_value(json_object_get(root, "result"))) {
        json_decref(root);
        return false;
    }

    *alarm = json_boolean_value(json_object_get(root, "alarm"));

    json_decref(root);
    return true;
}

bool RpcSecuritySensorsGet(unsigned unit, GList **sensors)
{
    char            buf[BUFFER_LEN_MAX];
    char            url[STR_LEN];
    json_error_t    error;
    size_t          index;
    json_t          *value;

    if (sensors == NULL) {
        return false;
    }

    if (unit == RPC_DEFAULT_UNIT) {
        for (GList *c = *SecuritySensorsGet(); c != NULL; c = c->next) {
            SecuritySensor *sensor = (SecuritySensor *)c->data;

            RpcSecuritySensor *s = (RpcSecuritySensor *)malloc(sizeof(RpcSecuritySensor));
            strncpy(s->name, sensor->name, SHORT_STR_LEN);
            s->detected = sensor->detected;
            switch (sensor->type) {
                case SECURITY_SENSOR_REED:
                    s->type = RPC_SECURITY_SENSOR_REED;
                    break;

                case SECURITY_SENSOR_MICRO_WAVE:
                    s->type = RPC_SECURITY_SENSOR_MICRO_WAVE;
                    break;

                case SECURITY_SENSOR_PIR:
                    s->type = RPC_SECURITY_SENSOR_PIR;
                    break;
            }

            *sensors = g_list_append(*sensors, s);
        }
        return true;
    }

    StackUnit *u = StackUnitGet(unit);
    if (u == NULL) {
        return false;
    }

    snprintf(url, STR_LEN, "http://%s:%d/api/%s/security?cmd=sensors_get", u->ip, u->port, SERVER_API_VER);
    memset(buf, 0x0, BUFFER_LEN_MAX);

    if (!WebClientRequest(WEB_REQ_GET, url, NULL, buf)) {
        return false;
    }

    json_t *root = json_loads(buf, 0, &error);
    if (root == NULL) {
        return false;
    }

    if (!json_boolean_value(json_object_get(root, "result"))) {
        json_decref(root);
        return false;
    }

    json_array_foreach(json_object_get(root, "sensors"), index, value) {
        RpcSecuritySensor *s = (RpcSecuritySensor *)malloc(sizeof(RpcSecuritySensor));

        strncpy(s->name, json_string_value(json_object_get(value, "name")), SHORT_STR_LEN);
        s->detected = json_boolean_value(json_object_get(value, "detected"));
        s->type = json_integer_value(json_object_get(value, "type"));

        *sensors = g_list_append(*sensors, s);
    }

    json_decref(root);
    return true;
}

/*********************************************************************/
/*                                                                   */
/*                           METEO FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

bool RpcMeteoSensorsGet(unsigned unit, GList **sensors)
{
    char            buf[BUFFER_LEN_MAX];
    char            url[STR_LEN];
    json_error_t    error;
    size_t          index;
    json_t          *value;

    if (sensors == NULL) {
        return false;
    }

    if (unit == RPC_DEFAULT_UNIT) {
        for (GList *c = *MeteoSensorsGet(); c != NULL; c = c->next) {
            MeteoSensor *sensor = (MeteoSensor *)c->data;

            RpcMeteoSensor *s = (RpcMeteoSensor *)malloc(sizeof(RpcMeteoSensor));
            strncpy(s->name, sensor->name, SHORT_STR_LEN);
            switch (sensor->type) {
                case METEO_SENSOR_DS18B20:
                    s->type = RPC_METEO_SENSOR_DS18B20;
                    s->ds18b20.temp = sensor->ds18b20.temp;
                    break;
            }

            *sensors = g_list_append(*sensors, (void *)s);
        }
        return true;
    }

    StackUnit *u = StackUnitGet(unit);
    if (u == NULL) {
        return false;
    }

    snprintf(url, STR_LEN, "http://%s:%d/api/%s/meteo?cmd=sensors_get", u->ip, u->port, SERVER_API_VER);
    memset(buf, 0x0, BUFFER_LEN_MAX);

    if (!WebClientRequest(WEB_REQ_GET, url, NULL, buf)) {
        return false;
    }

    json_t *root = json_loads(buf, 0, &error);
    if (root == NULL) {
        return false;
    }

    if (!json_boolean_value(json_object_get(root, "result"))) {
        json_decref(root);
        return false;
    }

    json_array_foreach(json_object_get(root, "sensors"), index, value) {
        RpcMeteoSensor *s = (RpcMeteoSensor *)malloc(sizeof(RpcMeteoSensor));

        strncpy(s->name, json_string_value(json_object_get(value, "name")), SHORT_STR_LEN);
        switch (json_integer_value(json_object_get(value, "pres"))) {
            case METEO_SENSOR_DS18B20:
                s->type = RPC_METEO_SENSOR_DS18B20;
                s->ds18b20.temp = (float)json_real_value(json_object_get(json_object_get(value, "ds18b20"), "temp"));
                break;
        }

        *sensors = g_list_append(*sensors, (void *)s);
    }

    json_decref(root);
    return true;
}

/*********************************************************************/
/*                                                                   */
/*                         SOCKET  FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

bool RpcSocketStatusSet(unsigned unit, const char *name, bool status)
{
    char            buf[BUFFER_LEN_MAX];
    char            url[STR_LEN];
    json_error_t    error;

    if (unit == RPC_DEFAULT_UNIT) {
        Socket *socket = SocketGet(name);
        if (socket == NULL) {
            return false;
        }
        return SocketStatusSet(socket, status, true);
    }

    StackUnit *u = StackUnitGet(unit);
    if (u == NULL) {
        return false;
    }

    snprintf(url, STR_LEN, "http://%s:%d/api/%s/socket?cmd=status_set&name=%s&status=%s",
            u->ip, u->port, SERVER_API_VER, name, (status == true) ? "true" : "false");
    memset(buf, 0x0, BUFFER_LEN_MAX);

    if (!WebClientRequest(WEB_REQ_GET, url, NULL, buf)) {
        return false;
    }

    json_t *root = json_loads(buf, 0, &error);
    if (root == NULL) {
        return false;
    }

    if (!json_boolean_value(json_object_get(root, "result"))) {
        json_decref(root);
        return false;
    }

    json_decref(root);
    return true;
}

bool RpcSocketsGet(unsigned unit, GList **sockets)
{
    char            buf[BUFFER_LEN_MAX];
    char            url[STR_LEN];
    json_error_t    error;
    size_t          index;
    json_t          *value;

    if (sockets == NULL) {
        return false;
    }

    if (unit == RPC_DEFAULT_UNIT) {
        for (GList *c = *SocketsGet(); c != NULL; c = c->next) {
            Socket *socket = (Socket *)c->data;

            RpcSocket *s = (RpcSocket *)malloc(sizeof(RpcSocket));
            strncpy(s->name, socket->name, SHORT_STR_LEN);
            s->status = socket->status;

            switch (socket->group) {
                case SOCKET_GROUP_LIGHT:
                    s->group = RPC_SOCKET_GROUP_LIGHT;
                    break;

                case SOCKET_GROUP_SOCKET:
                    s->group = RPC_SOCKET_GROUP_SOCKET;
                    break;
            }

            *sockets = g_list_append(*sockets, (void *)s);
        }
        return true;
    }

    StackUnit *u = StackUnitGet(unit);
    if (u == NULL) {
        return false;
    }

    snprintf(url, STR_LEN, "http://%s:%d/api/%s/socket?cmd=sockets_get", u->ip, u->port, SERVER_API_VER);
    memset(buf, 0x0, BUFFER_LEN_MAX);

    if (!WebClientRequest(WEB_REQ_GET, url, NULL, buf)) {
        return false;
    }

    json_t *root = json_loads(buf, 0, &error);
    if (root == NULL) {
        return false;
    }

    if (!json_boolean_value(json_object_get(root, "result"))) {
        json_decref(root);
        return false;
    }

    json_array_foreach(json_object_get(root, "sockets"), index, value) {
        RpcSocket *s = (RpcSocket *)malloc(sizeof(RpcSocket));

        strncpy(s->name, json_string_value(json_object_get(value, "name")), SHORT_STR_LEN);
        s->status = json_boolean_value(json_object_get(value, "status"));

        if (!strcmp(json_string_value(json_object_get(value, "group")), "light")) {
            s->group = RPC_SOCKET_GROUP_LIGHT;
        } else if (!strcmp(json_string_value(json_object_get(value, "group")), "socket")) {
            s->group = RPC_SOCKET_GROUP_SOCKET;
        }

        *sockets = g_list_append(*sockets, (void *)s);
    }

    json_decref(root);
    return true;
}

/*********************************************************************/
/*                                                                   */
/*                           TANK  FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

bool RpcTankStatusSet(unsigned unit, const char *name, bool status)
{
    char            buf[BUFFER_LEN_MAX];
    char            url[STR_LEN];
    json_error_t    error;

    if (unit == RPC_DEFAULT_UNIT) {
        Tank *tank = TankGet(name);
        if (tank == NULL) {
            return false;
        }
        return TankStatusSet(tank, status, true);
    }

    StackUnit *u = StackUnitGet(unit);
    if (u == NULL) {
        return false;
    }

    snprintf(url, STR_LEN, "http://%s:%d/api/%s/tank?cmd=status_set&name=%s&status=%s",
            u->ip, u->port, SERVER_API_VER, name, (status == true) ? "true" : "false");
    memset(buf, 0x0, BUFFER_LEN_MAX);

    if (!WebClientRequest(WEB_REQ_GET, url, NULL, buf)) {
        return false;
    }

    json_t *root = json_loads(buf, 0, &error);
    if (root == NULL) {
        return false;
    }

    if (!json_boolean_value(json_object_get(root, "result"))) {
        json_decref(root);
        return false;
    }

    json_decref(root);
    return true;
}

bool RpcTanksGet(unsigned unit, GList **tanks)
{
    char            buf[BUFFER_LEN_MAX];
    char            url[STR_LEN];
    json_error_t    error;
    size_t          index;
    json_t          *value;

    if (tanks == NULL) {
        return false;
    }

    if (unit == RPC_DEFAULT_UNIT) {
        for (GList *c = *TanksGet(); c != NULL; c = c->next) {
            Tank *tank = (Tank *)c->data;

            RpcTank *t = (RpcTank *)malloc(sizeof(RpcTank));
            strncpy(t->name, tank->name, SHORT_STR_LEN);
            t->status = tank->status;
            t->level = tank->level;
            t->pump = tank->pump;
            t->valve = tank->valve;

            *tanks = g_list_append(*tanks, (void *)t);
        }
        return true;
    }

    StackUnit *u = StackUnitGet(unit);
    if (u == NULL) {
        return false;
    }

    snprintf(url, STR_LEN, "http://%s:%d/api/%s/tank?cmd=tanks_get", u->ip, u->port, SERVER_API_VER);
    memset(buf, 0x0, BUFFER_LEN_MAX);

    if (!WebClientRequest(WEB_REQ_GET, url, NULL, buf)) {
        return false;
    }

    json_t *root = json_loads(buf, 0, &error);
    if (root == NULL) {
        return false;
    }

    if (!json_boolean_value(json_object_get(root, "result"))) {
        json_decref(root);
        return false;
    }

    json_array_foreach(json_object_get(root, "tanks"), index, value) {
        RpcTank *t = (RpcTank *)malloc(sizeof(RpcTank));

        strncpy(t->name, json_string_value(json_object_get(value, "name")), SHORT_STR_LEN);
        t->status = json_boolean_value(json_object_get(value, "status"));
        t->level = json_integer_value(json_object_get(value, "level"));
        t->pump = json_boolean_value(json_object_get(value, "pump"));
        t->valve = json_boolean_value(json_object_get(value, "valve"));

        *tanks = g_list_append(*tanks, (void *)t);
    }

    json_decref(root);
    return true;
}

bool RpcTankPumpSet(unsigned unit, const char *name, bool status)
{
    char            buf[BUFFER_LEN_MAX];
    char            url[STR_LEN];
    json_error_t    error;

    if (unit == RPC_DEFAULT_UNIT) {
        Tank *tank = TankGet(name);
        if (tank == NULL) {
            return false;
        }
        return TankPumpSet(tank, status);
    }

    StackUnit *u = StackUnitGet(unit);
    if (u == NULL) {
        return false;
    }

    snprintf(url, STR_LEN, "http://%s:%d/api/%s/tank?cmd=pump_set&name=%s&status=%s",
            u->ip, u->port, SERVER_API_VER, name, (status == true) ? "true" : "false");
    memset(buf, 0x0, BUFFER_LEN_MAX);

    if (!WebClientRequest(WEB_REQ_GET, url, NULL, buf)) {
        return false;
    }

    json_t *root = json_loads(buf, 0, &error);
    if (root == NULL) {
        return false;
    }

    if (!json_boolean_value(json_object_get(root, "result"))) {
        json_decref(root);
        return false;
    }

    json_decref(root);
    return true;
}

bool RpcTankValveSet(unsigned unit, const char *name, bool status)
{
    char            buf[BUFFER_LEN_MAX];
    char            url[STR_LEN];
    json_error_t    error;

    if (unit == RPC_DEFAULT_UNIT) {
        Tank *tank = TankGet(name);
        if (tank == NULL) {
            return false;
        }
        return TankValveSet(tank, status);
    }

    StackUnit *u = StackUnitGet(unit);
    if (u == NULL) {
        return false;
    }

    snprintf(url, STR_LEN, "http://%s:%d/api/%s/tank?cmd=valve_set&name=%s&status=%s",
            u->ip, u->port, SERVER_API_VER, name, (status == true) ? "true" : "false");
    memset(buf, 0x0, BUFFER_LEN_MAX);

    if (!WebClientRequest(WEB_REQ_GET, url, NULL, buf)) {
        return false;
    }

    json_t *root = json_loads(buf, 0, &error);
    if (root == NULL) {
        return false;
    }

    if (!json_boolean_value(json_object_get(root, "result"))) {
        json_decref(root);
        return false;
    }

    json_decref(root);
    return true;
}
