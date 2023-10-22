/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#include <glib.h>

#include <core/lcd.h>

#ifdef __ARM__
#include <wiringPi.h>
#endif

/*********************************************************************/
/*                                                                   */
/*                         PRIVATE VARIABLES                         */
/*                                                                   */
/*********************************************************************/

static GList    *lcds = NULL;
static int      lcd_fd = 0;

/*********************************************************************/
/*                                                                   */
/*                          PUBLIC FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

GList **LcdsGet()
{
    return &lcds;
}

bool LcdAdd(const LCD *lcd, char *err)
{
#ifdef __ARM__
    pinMode(lcd->rw, OUTPUT);
    digitalWrite(lcd->rw, LOW);
    pinMode(lcd->k, OUTPUT);
    digitalWrite(lcd->k, HIGH);

    for (int i = 0; i < LCD_INIT_RETRIES; i++)
    {
        lcd_fd = lcdInit(LCD_ROWS, LCD_COLS, LCD_BITS, lcd->rs, lcd->e, lcd->d4, lcd->d5, lcd->d6, lcd->d7, 0, 0, 0, 0);
        if (lcd_fd != 0)
            break;
        delay(100);
    }

    if (lcd_fd == 0) {
        return false;
    }
#endif

    lcds = g_list_append(lcds, (void *)lcd);
    return true;
}

bool LcdPrint(const LCD *lcd, char *text)
{
#ifdef __ARM__
    if (lcdPuts(lcd_fd, text) < 0) {
        return false;
    }
#endif
    return true;
}

bool LcdPosSet(const LCD *lcd, unsigned row, unsigned col)
{
#ifdef __ARM__
    if (lcdPosition(lcd_fd, col, row) < 0) {
        return false;
    }
#endif
    return true;
}

bool LcdClear(const LCD *lcd)
{
#ifdef __ARM__
    if (lcdClear(lcd_fd) < 0) {
        return false;
    }
#endif
    return true;
}
