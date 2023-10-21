/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#ifndef __LOG_H__
#define __LOG_H__

#include <stdbool.h>
#include <stdio.h>

typedef enum {
    LOG_TYPE_INFO,
    LOG_TYPE_WARN,
    LOG_TYPE_ERROR
} LogType;

bool Log(const LogType type, const char *module, const char *msg);

#define LogF(type, module, args...) \
    do { \
        char buf[EXT_STR_LEN]; \
        snprintf(buf, EXT_STR_LEN, ##args); \
        Log(type, module, buf); \
    } while(0)

#endif /* __LOG_H__ */