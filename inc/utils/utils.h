/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdbool.h>

#include <glib.h>

#define GPIO_NAME_STR_LEN   30
#define ERROR_STR_LEN       255
#define STR_LEN             255
#define EXT_STR_LEN         1024
#define SHORT_STR_LEN       50

typedef struct {
    char    name[SHORT_STR_LEN];
    char    value[SHORT_STR_LEN];
} UtilsReqParam;

/**
 * @brief Parse URI params from request
 *
 * @param url Request uri
 * @param params Out parsed params
 *
 * @return true/false as result of parsing
 */
bool UtilsURIParse(const char *url, GList **params);

#endif /* __UTILS_H__ */
