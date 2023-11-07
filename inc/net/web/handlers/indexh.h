/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#ifndef __INDEX_HANDLER_H__
#define __INDEX_HANDLER_H__

#include <stdbool.h>

#include <fcgiapp.h>
#include <glib-2.0/glib.h>

/**
 * @brief Manage index page
 *
 * @param req FastCGI request
 * @param params Request URI params
 *
 * @return true/false as result of processing request
 */
bool HandlerIndexProcess(FCGX_Request *req, GList **params);

#endif /* __INDEX_HANDLER_H__ */
