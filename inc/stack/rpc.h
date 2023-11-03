/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#ifndef __RPC_H__
#define __RPC_H__

#include <stdbool.h>

#include <utils/utils.h>

#define RPC_SECURITY_SENSOR_REED    0

typedef struct {
    char        name[SHORT_STR_LEN];
    unsigned    type;
    bool        detected;
} RpcSecuritySensor;

bool RpcSecurityStatusSet(unsigned unit, bool status);
bool RpcSecurityStatusGet(unsigned unit, bool *status);
bool RpcSecurityAlarmSet(unsigned unit, bool alarm);
bool RpcSecurityAlarmGet(unsigned unit, bool *alarm);
bool RpcSecuritySensorsGet(unsigned unit, GList **sensors);

typedef struct {
    char    name[SHORT_STR_LEN];
    float   temp;
    float   hum;
    float   pres;
} RpcMeteoSensor;

bool RpcMeteoSensorsGet(unsigned unit, GList **sensors);

typedef struct {
    char    name[SHORT_STR_LEN];
    bool    status;
} RpcSocket;

bool RpcSocketStatusSet(unsigned unit, const char *name, bool status);
bool RpcSocketStatusGet(unsigned unit, const char *name);
bool RpcSocketsGet(unsigned unit, GList **sockets);

#endif /* __RPC_H__ */
