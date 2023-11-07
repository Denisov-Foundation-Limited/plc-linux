/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#include <net/tgbot/tgmenu.h>

#include <glib-2.0/glib.h>

/*********************************************************************/
/*                                                                   */
/*                         PRIVATE VARIABLES                         */
/*                                                                   */
/*********************************************************************/

static GList *menus = NULL;

/*********************************************************************/
/*                                                                   */
/*                          PUBLIC FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

void TgMenuAdd(TgMenu *menu)
{
    menus = g_list_append(menus, (void *)menu);
}

void TgMenuLevelSet(unsigned from, TgMenuLevel level)
{
    for (GList *m = menus; m != NULL; m = m->next) {
        TgMenu *menu = (TgMenu *)m->data;
        if (menu->from == from) {
            menu->level = level;
            break;
        }
    }
}

void TgMenuDataSet(unsigned from, const char *data)
{
    for (GList *m = menus; m != NULL; m = m->next) {
        TgMenu *menu = (TgMenu *)m->data;
        if (menu->from == from) {
            strncpy(menu->data, data, STR_LEN);
            break;
        }
    }
}

const char *TgMenuDataGet(unsigned from)
{
    for (GList *m = menus; m != NULL; m = m->next) {
        TgMenu *menu = (TgMenu *)m->data;
        if (menu->from == from) {
            return menu->data;
        }
    }
    return NULL;
}

TgMenuLevel TgMenuLevelGet(unsigned from)
{
    for (GList *m = menus; m != NULL; m = m->next) {
        TgMenu *menu = (TgMenu *)m->data;
        if (menu->from == from) {
            return menu->level;
        }
    }
    return TG_MENU_LVL_MAIN;
}

void TgMenuUnitSet(unsigned from, StackUnit *unit)
{
    for (GList *m = menus; m != NULL; m = m->next) {
        TgMenu *menu = (TgMenu *)m->data;
        if (menu->from == from) {
            menu->unit = unit;
            break;
        }
    }
}

StackUnit *TgMenuUnitGet(unsigned from)
{
    for (GList *m = menus; m != NULL; m = m->next) {
        TgMenu *menu = (TgMenu *)m->data;
        if (menu->from == from) {
            return menu->unit;
        }
    }
    return 0; 
}

void TgMenuBack(unsigned from)
{
    for (GList *m = menus; m != NULL; m = m->next) {
        TgMenu *menu = (TgMenu *)m->data;
        if (menu->from == from) {

            switch (menu->level) {
                case TG_MENU_LVL_MAIN:
                    menu->level = TG_MENU_LVL_STACK_SELECT;
                    break;

                case TG_MENU_LVL_SOCKET:
                    menu->level = TG_MENU_LVL_MAIN;
                    break;

                case TG_MENU_LVL_SECURITY:
                    menu->level = TG_MENU_LVL_MAIN;
                    break;

                case TG_MENU_LVL_METEO:
                    menu->level = TG_MENU_LVL_MAIN;
                    break;
            }
            break;
        }
    }
}
