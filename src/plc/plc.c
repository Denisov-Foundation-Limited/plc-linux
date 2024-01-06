/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#include <plc/plc.h>
#include <utils/utils.h>
#include <utils/log.h>
#include <net/web/webserver.h>
#include <net/tgbot/tgbot.h>
#include <controllers/controllers.h>
#include <stack/stack.h>
#include <db/dbloader.h>
#include <plc/menu.h>

#include <threads.h>

/*********************************************************************/
/*                                                                   */
/*                         PRIVATE VARIABLES                         */
/*                                                                   */
/*********************************************************************/

static struct _Plc {
    GpioPin     *gpio[PLC_GPIO_MAX];
    unsigned    alarms;
    bool        buzzer;
    PlcTimeType time_type;
} Plc = {
    .gpio = {0},
    .alarms = 0x0,
    .buzzer = false,
    .time_type = PLC_TIME_LINUX
};

/*********************************************************************/
/*                                                                   */
/*                         PRIVATE FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

static int AlarmThread(void *data)
{
    bool    last = true;

    for (;;) {
        if (Plc.alarms != 0x0) {
            if (last) {
                last = false;
                GpioPinWrite(Plc.gpio[PLC_GPIO_ALARM_LED], true);
            } else {
                last = true;
                GpioPinWrite(Plc.gpio[PLC_GPIO_ALARM_LED], false);
            }
        }

        if (Plc.buzzer) {
            if (last) {
                GpioPinWrite(Plc.gpio[PLC_GPIO_BUZZER], true);
            } else {
                GpioPinWrite(Plc.gpio[PLC_GPIO_BUZZER], false);
            }
        }

        UtilsMsecSleep(500);
    }
    return 0;
}

static int BuzzerThread(void *data)
{
    PlcBuzzerType *type = (PlcBuzzerType *)data;

    switch (*type) {
        case PLC_BUZZER_LOOP:
            break;

        case PLC_BUZZER_SECURITY_ENTER:
            GpioPinWrite(Plc.gpio[PLC_GPIO_BUZZER], true);
            UtilsMsecSleep(300);
            GpioPinWrite(Plc.gpio[PLC_GPIO_BUZZER], false);
            break;

        case PLC_BUZZER_SECURITY_EXIT:
            for (unsigned i = 0; i < 2; i++) {
                GpioPinWrite(Plc.gpio[PLC_GPIO_BUZZER], true);
                UtilsMsecSleep(100);
                GpioPinWrite(Plc.gpio[PLC_GPIO_BUZZER], false);
                UtilsMsecSleep(100);
            }
            break;

        case PLC_BUZZER_TANK_EMPTY:
            for (unsigned i = 0; i < 4; i++) {
                GpioPinWrite(Plc.gpio[PLC_GPIO_BUZZER], true);
                UtilsSecSleep(1);
                GpioPinWrite(Plc.gpio[PLC_GPIO_BUZZER], false);
                UtilsSecSleep(2);
            }
            break;
    }

    free(type);
    return 0;
}

/*********************************************************************/
/*                                                                   */
/*                         PUBLIC  FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

void PlcTimeTypeSet(PlcTimeType type)
{
    Plc.time_type = type;
}

bool PlcTimeGet(PlcTime *time)
{
    if (Plc.time_type == PLC_TIME_LINUX) {
        struct tm *t = UtilsLinuxTimeGet();

        time->sec = t->tm_sec;
        time->min = t->tm_min;
        time->hour = t->tm_hour;
        time->day = t->tm_mday;
        time->dow = t->tm_wday;
        time->month = t->tm_mon;
        time->year = t->tm_year;

        return true;
    }
    return false;
}

void PlcGpioSet(PlcGpioType type, GpioPin *gpio)
{
    Plc.gpio[type] = gpio;
}

void PlcAlarmSet(PlcAlarmType type, bool status)
{
    if (status) {
        Plc.alarms |= (1 << type);
    } else {
        Plc.alarms &= ~(1 << type);
    }

    if (Plc.alarms == 0x0) {
        UtilsSecSleep(1);
        GpioPinWrite(Plc.gpio[PLC_GPIO_ALARM_LED], false);
    }
}

void PlcBuzzerRun(PlcBuzzerType type, bool status)
{
    thrd_t  bz_th;

    if (type == PLC_BUZZER_LOOP) {
        Plc.buzzer = status;
        if (!status) {
            UtilsSecSleep(1);
            GpioPinWrite(Plc.gpio[PLC_GPIO_BUZZER], false);
        }
    } else {
        PlcBuzzerType *t = (PlcBuzzerType *)malloc(sizeof(PlcBuzzerType));
        *t = type;
        thrd_create(&bz_th, &BuzzerThread, (void *)t);
        thrd_detach(bz_th);
    }
}

bool PlcStart()
{
    thrd_t  alrm_th;

    Log(LOG_TYPE_INFO, "PLC", "Starting Plc");

    thrd_create(&alrm_th, &AlarmThread, NULL);
    thrd_detach(alrm_th);

    Log(LOG_TYPE_INFO, "PLC", "Loading database states");

    if (!DatabaseLoaderLoad()) {
        Log(LOG_TYPE_ERROR, "PLC", "Failed to load Database");
        return -1;
    }

    Log(LOG_TYPE_INFO, "PLC", "Starting controllers");

    if (!ControllersStart()) {
        Log(LOG_TYPE_ERROR, "PLC", "Failed to start controllers");
        return -1;
    }

    Log(LOG_TYPE_INFO, "PLC", "Starting Stack monitoring");

    if (!StackStart()) {
        Log(LOG_TYPE_ERROR, "PLC", "Failed to start stack");
        return -1;
    }

    Log(LOG_TYPE_INFO, "PLC", "Starting Telegram bot");

    if (!TgBotStart()) {
        Log(LOG_TYPE_ERROR, "PLC", "Failed to start Telegram bot");
        return -1;
    }

    Log(LOG_TYPE_INFO, "PLC", "Starting PLC Menu");

    if (!MenuStart()) {
        Log(LOG_TYPE_ERROR, "PLC", "Failed to start PLC Menu");
        return -1;
    }

    Log(LOG_TYPE_INFO, "PLC", "Starting Web Server");

    if (!WebServerStart()) {
        Log(LOG_TYPE_ERROR, "PLC", "Failed to start web server");
        return -1;
    }

    Log(LOG_TYPE_INFO, "PLC", "Stopping controller");

    return true;
}
