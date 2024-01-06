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
    bool    ret = false;
    float   temp = 0;

    for (;;) {
        for (GList *s = Meteo.sensors; s != NULL; s = s->next) {
            MeteoSensor *sensor = (MeteoSensor *)s->data;

            for (unsigned i = 0; i < METEO_SENSOR_TRIES; i++) {
                switch (sensor->type) {
                    case METEO_SENSOR_DS18B20:
                        ret = OneWireTempRead(sensor->ds18b20.id, &temp);
                        if (ret) {
                            sensor->ds18b20.temp = temp;
                        }
                        break;
                }

                if (ret) {
                    break;
                } else {
                    UtilsSecSleep(2);
                }
            }

            if (!ret) {
                if (!sensor->error) {
                    sensor->error = true;
                    sensor->ds18b20.temp = METEO_BAD_VAL;
                    LogF(LOG_TYPE_ERROR, "METEO", "Failed to read temp sensor \"%s\"", sensor->name);
                }
            } else {
                if (sensor->error) {
                    sensor->error = false;
                    LogF(LOG_TYPE_ERROR, "METEO", "Successfully read temp sensor \"%s\"", sensor->name);
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

MeteoSensor *MeteoSensorNew(const char *name, MeteoSensorType type)
{
    MeteoSensor *sensor = (MeteoSensor *)malloc(sizeof(MeteoSensor));

    strncpy(sensor->name, name, SHORT_STR_LEN);
    sensor->type = type;
    sensor->error = false;
    sensor->ds18b20.temp = 0;

    return sensor;
}

bool MeteoControllerStart()
{
    thrd_t  sens_th;

    Log(LOG_TYPE_INFO, "METEO", "Starting Meteo controller");

    if (thrd_create(&sens_th, &SensorsThread, NULL) != thrd_success) {
        return false;
    }
    if (thrd_detach(sens_th) != thrd_success) {
        return false;
    }

    return true;
}

MeteoSensor *MeteoSensorGet(const char *name)
{
    for (GList *v = Meteo.sensors; v != NULL; v = v->next) {
        MeteoSensor *sensor = (MeteoSensor *)v->data;
        if (!strcmp(sensor->name, name)) {
            return sensor;
        }
    }
    return NULL;
}

GList **MeteoSensorsGet()
{
    return &Meteo.sensors;
}

void MeteoSensorAdd(MeteoSensor *sensor)
{
    Meteo.sensors = g_list_append(Meteo.sensors, (void *)sensor);
}
