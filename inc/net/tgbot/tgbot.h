/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#ifndef __TELEGRAM_BOT_H__
#define __TELEGRAM_BOT_H__

#include <stdbool.h>

#include <utils/utils.h>

#define TG_BOT_GET_UPDATES_URL  ""

typedef struct {
    char        name[SHORT_STR_LEN];
    unsigned    chat_id;
} TgBotUser;

/**
 * @brief Make new telegram user
 * 
 * @param name User name
 * @param id User chat id
 * 
 * @return TgBotUserobject
 */
TgBotUser *TgBotUserNew(const char *name, unsigned id);

/**
 * @brief Disable Telegram Bot for slave controllers 
 */
void TgBotDisable();

/**
 * @brief Add new telegram bot user
 * 
 * @param user New user struct
 */
void TgBotUserAdd(TgBotUser *user);

/**
 * @brief Set bot token
 */
void TgBotTokenSet(const char *bot_token);

/**
 * @brief Start telegram bot
 * 
 * @return True/False as result of starting Telegram Bot
 */
bool TgBotStart();

#endif /* __TELEGRAM_BOT_H__ */
