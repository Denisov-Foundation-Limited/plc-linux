/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#include <threads.h>

#include <controllers/meteo.h>
#include <utils/log.h>
#include <net/notifier.h>
#include <core/onewire.h>

/*********************************************************************/
/*                                                                   */
/*                         PRIVATE VARIABLES                         */
/*                                                                   */
/*********************************************************************/

static struct _Meteo {
    GList   *sensors;
} Meteo = {
    .sensors = NULL
};

/*********************************************************************/
/*                                                                   */
/*                         PRIVATE FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

static int SensorsThread(void *data)
{
    char    msg[STR_LEN];
    bool    ret = false;

    for (;;) {
        for (GList *s = Meteo.sensors; s != NULL; s = s->next) {
            MeteoSensor *sensor = (MeteoSensor *)s->data;

            switch (sensor->type) {
                case METEO_SENSOR_DS18B20:
                    ret = OneWireTempRead(sensor->id, &sensor->temp);
                    break;
            }

            if (!ret) {
                if (!sensor->error) {
                    sensor->error = true;
                    LogF(LOG_TYPE_ERROR, "METEO", "Failed to read temp sensor \"%s\"", sensor->name);
                    snprintf(msg, STR_LEN, "МЕТЕО+Датчик+%s+отключился", sensor->name);
                    if (!NotifierTelegramSend(msg)) {
                        Log(LOG_TYPE_ERROR, "METEO", "Failed to send telegram message");
                    }
                }
            } else {
                if (sensor->error) {
                    sensor->error = false;
                    LogF(LOG_TYPE_ERROR, "METEO", "Failed to read temp sensor \"%s\"", sensor->name);
                    snprintf(msg, STR_LEN, "МЕТЕО+Датчик+%s+заработал", sensor->name);
                    if (!NotifierTelegramSend(msg)) {
                        Log(LOG_TYPE_ERROR, "METEO", "Failed to send telegram message");
                    }
                }
            }
        }
        UtilsSecSleep(10);
    }
}

/*********************************************************************/
/*                                                                   */
/*                          PUBLIC FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

bool MeteoControllerStart()
{
    thrd_t  sens_th;

    Log(LOG_TYPE_INFO, "METEO", "Starting Meteo controller");

    thrd_create(&sens_th, &SensorsThread, NULL);
    return true;
}

GList **MeteoSensorsGet()
{
    return &Meteo.sensors;
}

void MeteoSensorAdd(MeteoSensor *sensor)
{
    Meteo.sensors = g_list_append(Meteo.sensors, (void *)sensor);
}
