/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#ifndef __NOTIFIER_H__
#define __NOTIFIER_H__

#include <stdbool.h>

/**
 * @brief Set telegram bot credentials
 * 
 * @param bot_id Telegram bot ID
 * @param chat_id Telegram current user chat
 */
void NotifierTelegramCredsSet(const char *bot_id, unsigned chat_id);

/**
 * @brief Set sms credentials
 * 
 * @param api SMS.ru API token
 * @param phone User's phone number
 */
void NotifierSmsCredsSet(const char *api, const char *phone);

/**
 * @brief Send telegram message to bot
 * 
 * @param msg Telegram message
 * 
 * @return true/false as result of sending message
 */
bool NotifierTelegramSend(const char *msg);

/**
 * @brief Send sms message to phone
 * 
 * @param msg Telegram message
 * 
 * @return true/false as result of sending message
 */
bool NotifierSmsSend(const char *msg);

#endif /* __NOTIFIER_H__ */
