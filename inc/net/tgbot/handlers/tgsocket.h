/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#ifndef __TG_SOCKET_H__
#define __TG_SOCKET_H__

#include <stdbool.h>

void TgSocketSelectProcess(const char *token, unsigned from, const char *message);

void TgSocketProcess(const char *token, unsigned from, const char *message);

#endif /* __TG_SOCKET_H__ */
