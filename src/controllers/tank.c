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

#include <stdlib.h>
#include <threads.h>

/*********************************************************************/
/*                                                                   */
/*                         PRIVATE VARIABLES                         */
/*                                                                   */
/*********************************************************************/

static struct _Tanks {
    GList *tanks;
} Tanks = {
    .tanks = NULL
};

/*********************************************************************/
/*                                                                   */
/*                         PRIVATE FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

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
                if (NotifyLevelCheck(tank, level_num)) {
                    char    msg[STR_LEN];

                    snprintf(msg, STR_LEN, "Бак+\"%s\":+Уровень+воды+%u%%", tank->name, level_num);
                    LogF(LOG_TYPE_INFO, "TANK", "Tank \"%s\" water level %u%%", tank->name, level_num);

                    if (!NotifierTelegramSend(msg)) {
                        Log(LOG_TYPE_ERROR, "TANK", "Failed to send level notify");
                    }
                }
            }

            printf("LEVEL: %d%%\n", level_num);

            tank->level = level_num;
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
                    Log(LOG_TYPE_ERROR, "TANK", "Failed to switch tank status");
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

TankLevel *TankLevelNew(TankLevelType type, unsigned percent, GpioPin *gpio, bool notify)
{
    TankLevel *level = (TankLevel *)malloc(sizeof(TankLevel));

    level->type = type;
    level->percent = percent;
    level->gpio = gpio;
    level->notify = notify;

    return level;
}

Tank *TankNew(const char *name)
{
    Tank *tank = (Tank *)malloc(sizeof(Tank));

    strncpy(tank->name, name, SHORT_STR_LEN);
    tank->level = TANK_LEVEL_DEFAULT;
    tank->levels = NULL;
    tank->status = false;

    return tank;
}

void TankGpioSet(Tank *tank, TankGpio id, GpioPin *gpio)
{
    tank->gpio[id] = gpio;
}

void TankNotifySet(Tank *tank, TankNotify id, bool status)
{
    tank->notify[id] = status;
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
    tank->status = status;
}

bool TankStatusGet(Tank *tank)
{
    return tank->status;
}

bool TankControllerStart()
{
    thrd_t  lvl_th, sts_th;

    Log(LOG_TYPE_INFO, "TANK", "Starting Tank controller");

    thrd_create(&lvl_th, &TankLevelsThread, NULL);
    thrd_create(&sts_th, &TankStatusThread, NULL);

    return true;
}
