/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#ifndef __LCD_H__
#define __LCD_H__

#include <stdbool.h>

#include <glib-2.0/glib.h>

#include <utils/utils.h>

#define LCD_INIT_RETRIES    3
#define LCD_ROWS            2
#define LCD_COLS            16
#define LCD_BITS            4

#define LCD_DEFAULT_TEXT_UP     "  FUTURE  CITY  "
#define LCD_DEFAULT_TEXT_DOWN   "       PLC      "

typedef struct {
    char        name[SHORT_STR_LEN];
    unsigned    rs;
    unsigned    rw;
    unsigned    e;
    unsigned    k;
    unsigned    d4;
    unsigned    d5;
    unsigned    d6;
    unsigned    d7;
} LCD;

/**
 * @brief Get all LCD modules
 */
GList **LcdsGet();

/**
 * @brief Add new I2C LCD module
 * 
 * @param lcd LCD module struct
 * 
 * @return true/false as result of initialization of module
 */
bool LcdAdd(const LCD *lcd);

/**
 * @brief Print text on LCD
 * 
 * @param lcd LCD module struct
 * @param text Text message
 */
void LcdPrint(const LCD *lcd, char *text);

/**
 * @brief Changing position of LCD text
 * 
 * @param lcd LCD module struct
 * @param row Display row
 * @param col Display column
 */
void LcdPosSet(const LCD *lcd, unsigned row, unsigned col);

/**
 * @brief Clear text on LCD
 * 
 * @param lcd LCD module struct
 */
void LcdClear(const LCD *lcd);

#endif /* __LCD_H__ */
