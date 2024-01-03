/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#include <plc/menu.h>
#include <core/lcd.h>
#include <utils/log.h>
#include <stack/rpc.h>
#include <plc/plc.h>

#include <threads.h>

/*********************************************************************/
/*                                                                   */
/*                         PRIVATE VARIABLES                         */
/*                                                                   */
/*********************************************************************/

static struct _Menu {
    unsigned    level;
    GpioPin     *gpio[MENU_GPIO_MAX];
    GList       *levels;
    LCD         *lcd;
    mtx_t       upd_mtx;
    bool        pressed;
} Menu = {
    .level = 0,
    .levels = NULL,
    .lcd = NULL,
    .pressed = false
};

/*********************************************************************/
/*                                                                   */
/*                         PRIVATE FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

static void MenuDataPrint(MenuValue *value)
{
    char    val[SHORT_STR_LEN];

    if (value->ctrl == MENU_CTRL_METEO) {
        int     temp = 0;

        if (value->meteo.sensor->type == METEO_SENSOR_DS18B20) {
            temp = (int)value->meteo.sensor->ds18b20.temp;
        }

        if (temp == METEO_BAD_VAL) {
            snprintf(val, SHORT_STR_LEN, "%s:err", value->alias);
        } else if (temp > 0) {
            if (temp < 10) {
                snprintf(val, SHORT_STR_LEN, "%s: +%d", value->alias, temp);
            } else {
                snprintf(val, SHORT_STR_LEN, "%s:+%d", value->alias, temp);
            }
        } else if (temp < 0) {
            if (temp > -10) {
                snprintf(val, SHORT_STR_LEN, "%s: %d", value->alias, temp);
            } else {
                snprintf(val, SHORT_STR_LEN, "%s:%d", value->alias, temp);
            }
        } else {
            snprintf(val, SHORT_STR_LEN, "%s:  %d", value->alias, temp);
        }
    } else if (value->ctrl == MENU_CTRL_TIME) {
        PlcTime time;

        if (PlcTimeGet(&time)) {
            if (time.hour < 10) {
                if (time.min < 10) {
                    snprintf(val, SHORT_STR_LEN, "0%u:0%u", time.hour, time.min);
                } else {
                    snprintf(val, SHORT_STR_LEN, "0%u:%u", time.hour, time.min);
                }
            } else {
                if (time.min < 10) {
                    snprintf(val, SHORT_STR_LEN, "%u:0%u", time.hour, time.min);
                } else {
                    snprintf(val, SHORT_STR_LEN, "%u:%u", time.hour, time.min);
                }
            }
        } else {
            snprintf(val, SHORT_STR_LEN, "ERR");
        }
    } else if (value->ctrl == MENU_CTRL_TANK) {
        if (value->tank.param == MENU_TANK_LEVEL) {
            unsigned lvl = value->tank.tank->level;
            if (lvl < 10) {
                snprintf(val, SHORT_STR_LEN, "%s:  %u%%", value->alias, lvl);
            } else if (lvl >= 10 && lvl < 100) {
                snprintf(val, SHORT_STR_LEN, "%s: %u%%", value->alias, lvl);
            } else {
                snprintf(val, SHORT_STR_LEN, "%s:%u%%", value->alias, lvl);
            }
        } else if (value->tank.param == MENU_TANK_PUMP) {
            bool status = value->tank.tank->pump;
            snprintf(val, SHORT_STR_LEN, "%s:%s", value->alias, (status == true) ? "O" : "X");
        } else if (value->tank.param == MENU_TANK_VALVE) {
            bool status = value->tank.tank->valve;
            snprintf(val, SHORT_STR_LEN, "%s:%s", value->alias, (status == true) ? "O" : "X");
        }
    } else if (value->ctrl == MENU_CTRL_SOCKET) {
        bool status = value->socket.sock->status;
        snprintf(val, SHORT_STR_LEN, "%s:%s", value->alias, (status == true) ? "O" : "X");
    } else if (value->ctrl == MENU_CTRL_LIGHT) {
        bool status = value->light.sock->status;
        snprintf(val, SHORT_STR_LEN, "%s:%s", value->alias, (status == true) ? "O" : "X");
    }

    LcdPrint(Menu.lcd, val);
}

static int DisplayThread(void *data)
{
    unsigned cur_lvl = 0;
    unsigned counter = 0;

    for (;;) {
        if (counter == 50 || Menu.pressed) {
            Menu.pressed = false;
            counter = 0;
            cur_lvl = 0;

            for (GList *l = Menu.levels; l != NULL; l = l->next) {
                MenuLevel *level = (MenuLevel *)l->data;
                if (cur_lvl == Menu.level) {
                    LcdClear(Menu.lcd);

                    if (strcmp(level->name, "main")) {
                        LcdPosSet(Menu.lcd, 0, 0);
                        LcdPrint(Menu.lcd, level->name);
                    }

                    for (GList *v = level->values; v != NULL; v = v->next) {
                        MenuValue *value = (MenuValue *)v->data;
                        LcdPosSet(Menu.lcd, value->row, value->col);
                        MenuDataPrint(value);
                    }
                }
                cur_lvl++;
            }
        }
        counter++;
        UtilsMsecSleep(100);
    }
    return 0;
}

static int ButtonsThread(void *data)
{
    bool state;

    for (;;) {
        if (!GpioPinRead(Menu.gpio[MENU_GPIO_UP], &state)) {
            LogF(LOG_TYPE_ERROR, "MENU", "Failed to read GPIO \"%s\"", Menu.gpio[MENU_GPIO_UP]->name);
        } else { 
            if (!state) {
                if (Menu.level < (g_list_length(Menu.levels) - 1)) {
                    Menu.level++;
                } else {
                    Menu.level = 0;
                }
                Menu.pressed = true;
                UtilsMsecSleep(800);
            }
        }

        if (!GpioPinRead(Menu.gpio[MENU_GPIO_DOWN], &state)) {
            LogF(LOG_TYPE_ERROR, "MENU", "Failed to read GPIO \"%s\"", Menu.gpio[MENU_GPIO_UP]->name);
        } else {
            if (!state) {
                if (Menu.level > 0) {
                    Menu.level--;
                } else {
                    Menu.level = g_list_length(Menu.levels) - 1;
                }
                Menu.pressed = true;
                UtilsMsecSleep(800);
            }
        }

        UtilsMsecSleep(200);
    }
    return 0;
}

/*********************************************************************/
/*                                                                   */
/*                         PUBLIC FUNCTIONS                          */
/*                                                                   */
/*********************************************************************/

void MenuGpioSet(MenuGpioType type, GpioPin *gpio)
{
    Menu.gpio[type] = gpio;
}

void MenuLcdSet(LCD *lcd)
{
    Menu.lcd = lcd;
}

MenuLevel *MenuLevelNew(const char *name)
{
    MenuLevel *level = (MenuLevel *)malloc(sizeof(MenuLevel));

    strncpy(level->name, name, SHORT_STR_LEN);
    level->values = NULL;

    return level;
}

MenuValue *MenuValueNew(unsigned row, unsigned col, const char *alias, MenuController ctrl)
{
    MenuValue *value = (MenuValue *)malloc(sizeof(MenuValue));

    value->row = row;
    value->col = col;
    strncpy(value->alias, alias, SHORT_STR_LEN);
    value->ctrl = ctrl;

    return value;
}

void MenuLevelAdd(MenuLevel *level)
{
    Menu.levels = g_list_append(Menu.levels, (void *)level);
}

void MenuValueAdd(MenuLevel *level, MenuValue *value)
{
    level->values = g_list_append(level->values, (void *)value);
}

bool MenuStart()
{
    thrd_t  btn_th, lcd_th;

    thrd_create(&btn_th, &ButtonsThread, NULL);
    thrd_detach(btn_th);
    thrd_create(&lcd_th, &DisplayThread, NULL);
    thrd_detach(lcd_th);

    return true;
}
