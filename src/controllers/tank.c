/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#include <controllers/tank.h>
#include <utils/log.h>
#include <net/notifier.h>
#include <db/database.h>
#include <plc/plc.h>

#include <stdlib.h>
#include <threads.h>

/*********************************************************************/
/*                                                                   */
/*                         PRIVATE VARIABLES                         */
/*                                                                   */
/*********************************************************************/

static struct _Tanks {
    GList   *tanks;
    mtx_t   sts_mtx;
} Tanks = {
    .tanks = NULL
};

/*********************************************************************/
/*                                                                   */
/*                         PRIVATE FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

static bool StatusSave(Tank *tank)
{
    Database    db;
    char        sql[STR_LEN];
    char        con[STR_LEN];

    if (!DatabaseOpen(&db, TANK_DB_FILE)) {
        DatabaseClose(&db);
        Log(LOG_TYPE_ERROR, "TANK", "Failed to load Tank database");
        return false;
    }

    snprintf(sql, STR_LEN, "status=%d", (int)tank->status);
    snprintf(con, STR_LEN, "name=\"%s\"", tank->name);

    if (!DatabaseUpdate(&db, "tank", sql, con)) {
        DatabaseClose(&db);
        Log(LOG_TYPE_ERROR, "TANK", "Failed to update Tank database");
        return false;
    }

    DatabaseClose(&db);

    return true;
}

static bool NotifyLevelCheck(Tank *tank, unsigned num)
{
    for (GList *l = tank->levels; l != NULL; l = l->next) {
        TankLevel *level = (TankLevel *)l->data;
        if (level->percent == num && level->notify) {
            return true;
        }
    }
    return false;
}

static void TankLevelProcess(Tank *tank)
{
    if (!tank->status) {
        return;
    }

    LogF(LOG_TYPE_INFO, "TANK", "Tank \"%s\" water level %u%%", tank->name,  tank->level);

    if (tank->level == TANK_LEVEL_PERCENT_MIN) {
        GpioPinWrite(tank->gpio[TANK_GPIO_PUMP], false);
        tank->pump = false;
        PlcAlarmSet(PLC_ALARM_TANK, true);
        PlcBuzzerRun(PLC_BUZZER_TANK_EMPTY, true);
    } else {
        PlcAlarmSet(PLC_ALARM_TANK, false);
        GpioPinWrite(tank->gpio[TANK_GPIO_PUMP], true);
        tank->pump = true;
    }

    if (tank->level == TANK_LEVEL_PERCENT_MAX) {
        GpioPinWrite(tank->gpio[TANK_GPIO_VALVE], false);
        tank->valve = false;
    } else {
        GpioPinWrite(tank->gpio[TANK_GPIO_VALVE], true);
        tank->valve = true;
    }

    LogF(LOG_TYPE_INFO, "TANK", "Tank \"%s\" valve %s", tank->name, (tank->valve == true) ? "openned" : "closed");
    LogF(LOG_TYPE_INFO, "TANK", "Tank \"%s\" pump %s", tank->name, (tank->pump == true) ? "enabled" : "disabled");

    if (NotifyLevelCheck(tank, tank->level)) {
        char    msg[STR_LEN];

        snprintf(msg, STR_LEN, "БАК+\"%s\":+уровень+воды+%u%%", tank->name,  tank->level);

        if (!NotifierTelegramSend(msg)) {
            Log(LOG_TYPE_ERROR, "TANK", "Failed to send level notify");
        }
    }
}

static int TankLevelsThread(void *data)
{
     for (;;) {
        for (GList *t = Tanks.tanks; t != NULL; t = t->next) {
            Tank *tank = (Tank *)t->data;
            unsigned level_num = 0;

            for (GList *l = tank->levels; l != NULL; l = l->next) {
                TankLevel *level = (TankLevel *)l->data;

                bool state = GpioPinRead(level->gpio);

                if (state) {
                    if (level->percent > level_num) {
                        level_num = level->percent;
                    }
                }

                level->state = state;
            }

            if (tank->level != level_num) {
                tank->level = level_num;

                TankLevelProcess(tank);
            }
        }
        UtilsSecSleep(1);
    }
}

static int TankStatusThread(void *data)
{
     for (;;) {
        bool pressed = false;

        for (GList *t = Tanks.tanks; t != NULL; t = t->next) {
            Tank *tank = (Tank *)t->data;

            if (GpioPinRead(tank->gpio[TANK_GPIO_STATUS_BUTTON])) {
                pressed = true;
                if (!TankStatusSet(tank, !TankStatusGet(tank), true)) {
                    LogF(LOG_TYPE_ERROR, "TANK", "Failed to switch tank \"%s\" status", tank->name);
                }
            }
        }

        if (pressed) {
            UtilsMsecSleep(800);
        }

        UtilsMsecSleep(200);
     }
}

/*********************************************************************/
/*                                                                   */
/*                         PUBLIC  FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

TankLevel *TankLevelNew(unsigned percent, GpioPin *gpio, bool notify)
{
    TankLevel *level = (TankLevel *)malloc(sizeof(TankLevel));

    level->percent = percent;
    level->gpio = gpio;
    level->notify = notify;

    return level;
}

Tank *TankNew(const char *name)
{
    Tank *tank = (Tank *)malloc(sizeof(Tank));

    strncpy(tank->name, name, SHORT_STR_LEN);
    tank->level = TANK_LEVEL_PERCENT_DEFAULT;
    tank->levels = NULL;
    tank->status = false;
    tank->pump = false;
    tank->valve = false;

    return tank;
}

GList **TanksGet()
{
    return &Tanks.tanks;
}

void TankGpioSet(Tank *tank, TankGpio id, GpioPin *gpio)
{
    tank->gpio[id] = gpio;
}

void TankLevelAdd(Tank *tank, TankLevel *level)
{
    tank->levels = g_list_append(tank->levels, (void *)level);
}

void TankAdd(Tank *tank)
{
    Tanks.tanks = g_list_append(Tanks.tanks, (void *)tank);
}

bool TankStatusSet(Tank *tank, bool status, bool save)
{
    if (tank->status != status) {
        mtx_lock(&Tanks.sts_mtx);

        LogF(LOG_TYPE_INFO, "TANK", "Tank \"%s\" water control %s", tank->name, (status == true) ? "enabled" : "disabled");

        tank->status = status;
        GpioPinWrite(tank->gpio[TANK_GPIO_STATUS_LED], status);

        if (!status) {
            GpioPinWrite(tank->gpio[TANK_GPIO_PUMP], false);
            GpioPinWrite(tank->gpio[TANK_GPIO_VALVE], false);
            tank->valve = false;
            tank->pump = false;
            PlcAlarmSet(PLC_ALARM_TANK, false);
        }

        if (save) {
            StatusSave(tank);
        }

        mtx_unlock(&Tanks.sts_mtx);

        TankLevelProcess(tank);
    }
    return true;
}

Tank *TankGet(const char *name)
{
    for (GList *t = Tanks.tanks; t != NULL; t = t->next) {
        Tank *tank = (Tank *)t->data;
        if (!strcmp(tank->name, name)) {
            return tank;
        }
    }
    return NULL;
}

bool TankStatusGet(Tank *tank)
{
    return tank->status;
}

bool TankPumpSet(Tank *tank, bool status)
{
    if (!TankStatusSet(tank, false, true)) {
        LogF(LOG_TYPE_ERROR, "TANK", "Failed to disable tank \"%s\" status", tank->name);
        return false;
    }

    GpioPinWrite(tank->gpio[TANK_GPIO_PUMP], status);
    tank->pump = status;
    LogF(LOG_TYPE_INFO, "TANK", "Tank \"%s\" pump %s", tank->name, (status == true) ? "enabled" : "disabled");

    return true;
}

bool TankValveSet(Tank *tank, bool status)
{
    if (!TankStatusSet(tank, false, true)) {
        LogF(LOG_TYPE_ERROR, "TANK", "Failed to disable tank \"%s\" status", tank->name);
        return false;
    }

    GpioPinWrite(tank->gpio[TANK_GPIO_VALVE], status);
    tank->valve = status;
    LogF(LOG_TYPE_INFO, "TANK", "Tank \"%s\" valve %s", tank->name, (status == true) ? "openned" : "closed");

    return true;
}

bool TankControllerStart()
{
    thrd_t  lvl_th, sts_th;

    Log(LOG_TYPE_INFO, "TANK", "Starting Tank controller");

    thrd_create(&lvl_th, &TankLevelsThread, NULL);
    thrd_detach(lvl_th);
    thrd_create(&sts_th, &TankStatusThread, NULL);
    thrd_detach(sts_th);

    return true;
}
