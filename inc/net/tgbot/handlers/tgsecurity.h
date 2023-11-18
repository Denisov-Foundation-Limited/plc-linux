/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#ifndef __TG_SECURITY_H__
#define __TG_SECURITY_H__

#include <stdbool.h>

void TgSecurityProcess(const char *token, unsigned from, const char *message);

#endif /* __TG_SECURITY_H__ */
