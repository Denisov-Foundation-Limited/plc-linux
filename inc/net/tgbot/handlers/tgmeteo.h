/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#ifndef __TG_METEO_H__
#define __TG_METEO_H__

#include <stdbool.h>

/**
 * @brief Process Meteo Menu commands
 * 
 * @param token Telegram Bot uniq token
 * @param from From user id
 * @param message Telegram message 
 */
void TgMeteoProcess(const char *token, unsigned from, const char *message);

#endif /* __TG_METEO_H__ */
