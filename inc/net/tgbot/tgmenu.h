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

#include <stack/stack.h>

typedef enum {
    TG_MENU_LVL_MAIN,
    TG_MENU_LVL_METEO,
    TG_MENU_LVL_SECURITY_SELECT,
    TG_MENU_LVL_SECURITY,
    TG_MENU_LVL_SOCKET_SELECT,
    TG_MENU_LVL_SOCKET
} TgMenuLevel;

typedef struct {
    unsigned    from;
    TgMenuLevel level;
    TgMenuLevel prev;
    StackUnit   *unit;
    char        data[STR_LEN];
} TgMenu;

/**
 * @brief Add menu for user
 * 
 * @param menu New user's menu
 */
void TgMenuAdd(TgMenu *menu);

/**
 * @brief Get current menu level for user
 * 
 * @param from User ID
 * 
 * @return Current menu level
 */
TgMenuLevel TgMenuLevelGet(unsigned from);

/**
 * @brief Set unit for uniq User menu
 * 
 * @param from User ID
 * @param unit Stack Unit pointer
 */
void TgMenuUnitSet(unsigned from, StackUnit *unit);

/**
 * @brief Get unit for uniq User menu
 * 
 * @param from User ID
 * 
 * @return Stack Unit pointer
 */
StackUnit *TgMenuUnitGet(unsigned from);

/**
 * @brief Set additional data for uniq user
 * 
 * @param from User ID
 * @param data Additional data
 */
void TgMenuDataSet(unsigned from, const char *data);

/**
 * @brief Get additional data for uniq user
 * 
 * @param from User ID
 * 
 * @return Additional data
 */
const char *TgMenuDataGet(unsigned from);

/**
 * @brief Set new menu for user
 * 
 * @param from User ID
 * @param level New user menu
 */
void TgMenuLevelSet(unsigned from, TgMenuLevel level);

/**
 * @brief Set previous menu for user
 * 
 * @param from User ID 
 */
void TgMenuBack(unsigned from);

#endif /* __TG_MENU_H__ */
