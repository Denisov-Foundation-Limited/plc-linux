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

#include <utils/utils.h>

typedef struct {
    char        name[STR_LEN];
    unsigned    rs;
    unsigned    rw;
    unsigned    e;
    unsigned    k;
    unsigned    d4;
    unsigned    d5;
    unsigned    d6;
    unsigned    d7;
} LCD;

bool LcdAdd(const LCD *ext, char *err);

#endif /* __LCD_H__ */
