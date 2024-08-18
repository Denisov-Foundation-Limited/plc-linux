/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#include <controllers/meteo.h>
#include <controllers/therm.h>
#include <utils/log.h>
#include <net/notifier.h>
#include <db/database.h>
#include <plc/plc.h>
#include <core/gpio.h>

#include <stdlib.h>
#include <threads.h>

/*********************************************************************/
/*                                                                   */
/*                         PRIVATE VARIABLES                         */
/*                                                                   */
/*********************************************************************/

static struct _Tanks {
    GList   *therms;
    mtx_t   sts_mtx;
} Tanks = {
    .therms = NULL
};

/*********************************************************************/
/*                                                                   */
/*                         PRIVATE FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

static int ThermsThread(void *data)
{
    for (;;) {
        MeteoSensor *sensor = MeteoSensorGet("ТеплицаМ");
        GpioPin *pin = GpioPinGet("rly-green-m");

        if (sensor->ds18b20.temp > 28) {
            GpioPinWrite(pin, true);
        }

        if (sensor->ds18b20.temp < 28) {
            GpioPinWrite(pin, false);
        }

        UtilsSecSleep(1);
    }
}

/*********************************************************************/
/*                                                                   */
/*                         PUBLIC  FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

bool ThermControllerStart()
{
    thrd_t  sts_th;

    Log(LOG_TYPE_INFO, "THERM", "Starting therms controller");

    if (thrd_create(&sts_th, &ThermsThread, NULL) != thrd_success) {
        return false;
    }
    if (thrd_detach(sts_th) != thrd_success) {
        return false;
    }

    return true;
}
