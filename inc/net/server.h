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

/**
 * @brief Starting FastCGI web server
 * 
 * @param host Server host
 * @param port Server port
 * 
 * @return true/false as result of starting server
 */
bool WebServerStart(const char *host, unsigned port);

#endif /* __WEB_SERVER_H__ */
