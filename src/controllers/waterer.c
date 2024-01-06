/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#include <controllers/waterer.h>
#include <utils/log.h>
#include <net/notifier.h>
#include <db/database.h>

#include <threads.h>

/*********************************************************************/
/*                                                                   */
/*                          PRIVATE VARIABLES                        */
/*                                                                   */
/*********************************************************************/

static struct {
    GList   *waterers;
    mtx_t   sts_mtx;
} Watering = {
    .waterers = NULL
};

/*********************************************************************/
/*                                                                   */
/*                          PRIVATE FUNCTIONS                        */
/*                                                                   */
/*********************************************************************/

static bool StatusSave(Waterer *wtr)
{
    Database    db;
    char        sql[STR_LEN];
    char        con[STR_LEN];

    if (!DatabaseOpen(&db, WATERER_DB_FILE)) {
        DatabaseClose(&db);
        Log(LOG_TYPE_ERROR, "WATERER", "Failed to load Tank database");
        return false;
    }

    snprintf(sql, STR_LEN, "status=%d", (int)wtr->status);
    snprintf(con, STR_LEN, "name=\"%s\"", wtr->name);

    if (!DatabaseUpdate(&db, "waterer", sql, con)) {
        DatabaseClose(&db);
        Log(LOG_TYPE_ERROR, "WATERER", "Failed to update Waterer database");
        return false;
    }

    DatabaseClose(&db);

    return true;
}

static int WatererThread(void *data)
{
    PlcTime now;

    for (;;) {
        PlcTimeGet(&now);

        for (GList *w = Watering.waterers; w != NULL; w = w->next) {
            Waterer *wtr = (Waterer *)w->data;

            for (GList *t = Watering.waterers; t != NULL; t = t->next) {
                WateringTime *tm = (WateringTime *)t->data;

                if (tm->state != wtr->valve) {
                    if (tm->time.dow == now.dow && tm->time.hour == now.hour && tm->time.min == now.min && wtr->status) {
                        GpioPinWrite(wtr->gpio[WATERER_GPIO_VALVE], tm->state);
                        wtr->valve = tm->state;
                        LogF(LOG_TYPE_INFO, "WATERER", "Waterer \"%s\" valve %s", wtr->name, (tm->state == true) ? "openned" : "closed");

                        if (tm->notify) {
                            char    msg[STR_LEN];

                            snprintf(msg, STR_LEN, "ПОЛИВ+\"%s\":+кран+%s", wtr->name, (wtr->valve == true) ? "открыт" : "закрыт");

                            if (!NotifierTelegramSend(msg)) {
                                Log(LOG_TYPE_ERROR, "WATERER", "Failed to send watere notify");
                            }
                        }
                    }
                }
            }
        }

        UtilsSecSleep(1);
    }
    return 0;
}

static int ButtonsThread(void *data)
{
    bool state;

    for (;;) {
        bool pressed = false;

        for (GList *w = Watering.waterers; w != NULL; w = w->next) {
            Waterer *wtr = (Waterer *)w->data;

            if (!GpioPinRead(wtr->gpio[WATERER_GPIO_STATUS_BUTTON], &state)) {
                LogF(LOG_TYPE_ERROR, "WATERER", "Failed to read GPIO \"%s\"", wtr->gpio[WATERER_GPIO_STATUS_BUTTON]->name);
            } else {
                if (state) {
                    pressed = true;
                    if (!WatererStatusSet(wtr, !wtr->status, true)) {
                        LogF(LOG_TYPE_ERROR, "WATERER", "Failed to switch Waterer \"%s\" status", wtr->name);
                    }
                }
            }
        }

        if (pressed) {
            UtilsMsecSleep(800);
        }

        UtilsMsecSleep(200);
    }
    return 0;
}

/*********************************************************************/
/*                                                                   */
/*                          PUBLIC FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

Waterer *WatererNew(const char *name)
{
    Waterer *wtr = (Waterer *)malloc(sizeof(Waterer));

    strncpy(wtr->name, name, SHORT_STR_LEN);
    wtr->status = false;
    wtr->times = NULL;
    wtr->valve = false;

    return wtr;
}

Waterer *WatererGet(const char *name)
{
    for (GList *s = Watering.waterers; s != NULL; s = s->next) {
        Waterer *wtr = (Waterer *)s->data;
        if (!strcmp(wtr->name, name)) {
            return wtr;
        }
    }
    return NULL;
}

WateringTime *WateringTimeNew(PlcTime time, bool state, bool notify)
{
    WateringTime *t = (WateringTime *)malloc(sizeof(WateringTime));

    t->time = time;
    t->state = state;
    t->notify = notify;

    return t;
}

void WatererAdd(Waterer *wtr)
{
    Watering.waterers = g_list_append(Watering.waterers, (void *)wtr);
}

void WatererTimeAdd(Waterer *wtr, WateringTime *tm)
{
    wtr->times = g_list_append(wtr->times, (void *)tm);
}

GList **WaterersGet()
{
    return &Watering.waterers;
}

void WatererGpioSet(Waterer *wtr, WatererGpioType type, GpioPin *gpio)
{
    wtr->gpio[type] = gpio;
}

bool WatererStatusSet(Waterer *wtr, bool status, bool save)
{
    if (wtr->status != status) {
        mtx_lock(&Watering.sts_mtx);

        LogF(LOG_TYPE_INFO, "WATERER", "Waterer \"%s\" status %s", wtr->name, (status == true) ? "enabled" : "disabled");

        wtr->status = status;
        GpioPinWrite(wtr->gpio[WATERER_GPIO_STATUS_LED], status);

        if (!status) {
            GpioPinWrite(wtr->gpio[WATERER_GPIO_VALVE], false);
            wtr->valve = false;
        }

        if (save) {
            StatusSave(wtr);
        }

        mtx_unlock(&Watering.sts_mtx);
    }
    return true;
}

bool WatererValveSet(Waterer *wtr, bool status)
{
    if (!WatererStatusSet(wtr, false, true)) {
        LogF(LOG_TYPE_ERROR, "WATERER", "Failed to disable Waterer \"%s\" status", wtr->name);
        return false;
    }

    GpioPinWrite(wtr->gpio[WATERER_GPIO_VALVE], status);
    wtr->valve = status;
    LogF(LOG_TYPE_INFO, "WATERER", "Waterer \"%s\" valve %s", wtr->name, (status == true) ? "openned" : "closed");

    return true;
}

bool WatererControllerStart()
{
    thrd_t  wtr_th, bth_th;

    if (g_list_length(Watering.waterers) == 0) {
        return true;
    }

    Log(LOG_TYPE_INFO, "WATERER", "Starting Waterer controller");

    if (thrd_create(&wtr_th, &WatererThread, NULL) != thrd_success) {
        return false;
    }

    if (thrd_detach(wtr_th) != thrd_success) {
        return false;
    }

    if (thrd_create(&bth_th, &ButtonsThread, NULL) != thrd_success) {
        return false;
    }

    if (thrd_detach(bth_th) != thrd_success) {
        return false;
    }

    return true;
}
