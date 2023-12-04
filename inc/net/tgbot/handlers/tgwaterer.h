/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#ifndef __TG_WATERER_H__
#define __TG_WATERER_H__

#include <stdbool.h>

/**
 * @brief Process Waterer stack unit selection Menu commands
 * 
 * @param token Telegram Bot uniq token
 * @param from From user id
 * @param message Telegram message 
 */
void TgWatererStackSelectProcess(const char *token, unsigned from, const char *message);

/**
 * @brief Process Waterer controller selection Menu commands
 * 
 * @param token Telegram Bot uniq token
 * @param from From user id
 * @param message Telegram message 
 */
void TgWatererSelectProcess(const char *token, unsigned from, const char *message);

/**
 * @brief Process Waterer controller Menu commands
 * 
 * @param token Telegram Bot uniq token
 * @param from From user id
 * @param message Telegram message 
 */
void TgWatererProcess(const char *token, unsigned from, const char *message);

#endif /* __TG_WATERER_H__ */
