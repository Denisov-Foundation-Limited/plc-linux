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

#include <fcgiapp.h>
#include <jansson.h>

typedef enum {
    RESPONSE_TYPE_OK,
    RESPONSE_TYPE_FAIL
} ResponseType;

/**
 * @brief Send FastCGI response
 *
 * @param type Response type: Success or fail
 * @param req Request struct
 * @param root Json response
 *
 * @return Returns true/false as result of sending response
 */
bool ResponseSend(ResponseType type, FCGX_Request *req, json_t *root);

#endif /* __RESPONSE_H__ */
