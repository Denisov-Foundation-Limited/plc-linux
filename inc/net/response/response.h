/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#ifndef __RESPONSE_H__
#define __RESPONSE_H__

#include <stdbool.h>
#include <stdio.h>

#include <fcgiapp.h>
#include <jansson.h>

#include <utils/utils.h>

/**
 * @brief Send FastCGI fail response
 *
 * @param req Request struct
 * @param module App module
 * @param error Error text message
 *
 * @return Returns true/false as result of sending response
 */
bool ResponseFailSend(FCGX_Request *req, const char *module, const char *error);

#define ResponseFailSendF(type, module, args...) \
    do { \
        char buf[EXT_STR_LEN]; \
        snprintf(buf, EXT_STR_LEN, ##args); \
        return ResponseFailSend(req, module, buf); \
    } while(0)

/**
 * @brief Send FastCGI OK response
 *
 * @param req Request struct
 * @param root Json response
 *
 * @return Returns true/false as result of sending response
 */
bool ResponseOkSend(FCGX_Request *req, json_t *root);

#endif /* __RESPONSE_H__ */
