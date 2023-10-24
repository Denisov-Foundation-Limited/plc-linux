/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#ifndef __WEB_SERVER_H__
#define __WEB_SERVER_H__

#include <stdbool.h>

#define SERVER_API_VER  "/api/v1"

/**
 * @brief Set server credentials
 * 
 * @param host Server host
 * @param port Server port
 */
void WebServerCredsSet(const char *host, unsigned port);

/**
 * @brief Starting FastCGI web server
 * 
 * @return true/false as result of starting server
 */
bool WebServerStart();

#endif /* __WEB_SERVER_H__ */
