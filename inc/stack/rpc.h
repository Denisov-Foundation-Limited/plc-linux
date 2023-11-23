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

/*********************************************************************/
/*                                                                   */
/*                           STACK MANAGMENT                         */
/*                                                                   */
/*********************************************************************/

#define RPC_DEFAULT_UNIT    0

bool RpcUnitStatusCheck(unsigned unit);

/*********************************************************************/
/*                                                                   */
/*                         SECURITY FUNCTIONS                        */
/*                                                                   */
/*********************************************************************/

typedef enum {
    RPC_SECURITY_SENSOR_REED,
    RPC_SECURITY_SENSOR_MICRO_WAVE,
    RPC_SECURITY_SENSOR_PIR
} RpcSecuritySensorType;

typedef struct {
    char                    name[SHORT_STR_LEN];
    RpcSecuritySensorType   type;
    bool                    detected;
} RpcSecuritySensor;

bool RpcSecurityStatusSet(unsigned unit, bool status);
bool RpcSecurityStatusGet(unsigned unit, bool *status);
bool RpcSecurityAlarmSet(unsigned unit, bool alarm);
bool RpcSecurityAlarmGet(unsigned unit, bool *alarm);
bool RpcSecuritySensorsGet(unsigned unit, GList **sensors);

/*********************************************************************/
/*                                                                   */
/*                           METEO FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

typedef enum {
    RPC_METEO_SENSOR_DS18B20
} RpcMeteoSensorType;

typedef struct {
    float   temp;
} RpcMeteoDs18b20;

typedef struct {
    char                name[SHORT_STR_LEN];
    RpcMeteoSensorType  type;
    RpcMeteoDs18b20     ds18b20;
} RpcMeteoSensor;

bool RpcMeteoSensorsGet(unsigned unit, GList **sensors);

/*********************************************************************/
/*                                                                   */
/*                         SOCKET  FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

typedef enum {
    RPC_SOCKET_GROUP_LIGHT,
    RPC_SOCKET_GROUP_SOCKET
} RpcSocketGroup;

typedef struct {
    char            name[SHORT_STR_LEN];
    RpcSocketGroup  group;
    bool            status;
} RpcSocket;

bool RpcSocketStatusSet(unsigned unit, const char *name, bool status);
bool RpcSocketsGet(unsigned unit, GList **sockets);

/*********************************************************************/
/*                                                                   */
/*                           TANK  FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

typedef struct {
    char        name[SHORT_STR_LEN];
    bool        status;
    unsigned    level;
    bool        pump;
    bool        valve;
} RpcTank;

bool RpcTankStatusSet(unsigned unit, const char *name, bool status);
bool RpcTankPumpSet(unsigned unit, const char *name, bool status);
bool RpcTankValveSet(unsigned unit, const char *name, bool status);
bool RpcTanksGet(unsigned unit, GList **tanks);

/*********************************************************************/
/*                                                                   */
/*                         CAMERA  FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

typedef struct {
    char    name[SHORT_STR_LEN];
} RpcCamera;

bool RpcCameraPhotoSave(unsigned unit, const char *name, const char *filename);
bool RpcCamerasGet(unsigned unit, GList **cams);
bool RpcCameraPathGet(unsigned unit, char *path);

#endif /* __RPC_H__ */
