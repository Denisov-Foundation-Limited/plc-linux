/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#ifndef __TG_TANK_H__
#define __TG_TANK_H__

#include <stdbool.h>

/**
 * @brief Process Tank stack unit selection Menu commands
 * 
 * @param token Telegram Bot uniq token
 * @param from From user id
 * @param message Telegram message 
 */
void TgTankStackSelectProcess(const char *token, unsigned from, const char *message);

/**
 * @brief Process Tank controller selection Menu commands
 * 
 * @param token Telegram Bot uniq token
 * @param from From user id
 * @param message Telegram message 
 */
void TgTankSelectProcess(const char *token, unsigned from, const char *message);

/**
 * @brief Process Tank controller Menu commands
 * 
 * @param token Telegram Bot uniq token
 * @param from From user id
 * @param message Telegram message 
 */
void TgTankProcess(const char *token, unsigned from, const char *message);

#endif /* __TG_TANK_H__ */
