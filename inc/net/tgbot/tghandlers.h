/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#ifndef __TG_HANDLERS_H__
#define __TG_HANDLERS_H__

/**
 * @brief Main menu of bot
 * 
 * @param token Telegram bot token
 * @param from User's ID
 * @param message input message
 */
void MainMenuProcess(const char *token, unsigned from, const char *message);

/**
 * @brief Meteo menu of bot
 * 
 * @param token Telegram bot token
 * @param from User's ID
 * @param message input message
 */
void MeteoMenuProcess(const char *token, unsigned from, const char *message);

/**
 * @brief Stack menu of bot
 * 
 * @param token Telegram bot token
 * @param from User's ID
 * @param message input message
 */
void StackSelectMenuProcess(const char *token, unsigned from, const char *message);


void SocketSelectMenuProcess(const char *token, unsigned from, const char *message);

/**
 * @brief Socket menu of bot
 * 
 * @param token Telegram bot token
 * @param from User's ID
 * @param message input message
 */
void SocketMenuProcess(const char *token, unsigned from, const char *message);

/**
 * @brief Security menu of bot
 * 
 * @param token Telegram bot token
 * @param from User's ID
 * @param message input message
 */
void SecurityMenuProcess(const char *token, unsigned from, const char *message);

#endif /* __TG_HANDLERS_H__ */
