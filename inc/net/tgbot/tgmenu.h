/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#ifndef __TG_MENU_H__
#define __TG_MENU_H__

#include <utils/utils.h>

typedef enum {
    TG_MENU_LVL_MAIN,
    TG_MENU_LVL_STACK_SELECT,
    TG_MENU_LVL_METEO,
    TG_MENU_LVL_SECURITY_SELECT,
    TG_MENU_LVL_SECURITY,
    TG_MENU_LVL_SOCKET
} TgMenuLevel;

typedef struct {
    unsigned    from;
    TgMenuLevel level;
    unsigned    unit;
    char        data[STR_LEN];
} TgMenu;

void TgMenuAdd(TgMenu *menu);

TgMenuLevel TgMenuLevelGet(unsigned from);

void TgMenuUnitSet(unsigned from, unsigned unit);

unsigned TgMenuUnitGet(unsigned from);

void TgMenuDataSet(unsigned from, const char *data);

const char *TgMenuDataGet(unsigned from);

void TgMenuLevelSet(unsigned from, TgMenuLevel level);

void TgMenuBack(unsigned from);

#endif /* __TG_MENU_H__ */
