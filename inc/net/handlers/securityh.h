/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#ifndef __SECURITY_HANDLER_H__
#define __SECURITY_HANDLER_H__

#include <stdbool.h>

#include <fcgiapp.h>
#include <glib.h>

/**
 * @brief Manage security controllers
 *
 * @param req FastCGI request
 * @param params Request URI params
 *
 * @return true/false as result of processing request
 */
bool HandlerSecurity(FCGX_Request *req, GList **params);

#endif /* __SECURITY_HANDLER_H__ */
