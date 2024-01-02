/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#ifndef __WEB_CLIENT_H__
#define __WEB_CLIENT_H__

#include <stdbool.h>

typedef enum {
    WEB_REQ_GET,
    WEB_REQ_POST
} WebRequestType;

/**
 * @brief HTTP request
 * 
 * @param type Web request type GET/POST
 * @param url Request URL
 * @param post Post fields or NULL
 * @param out Output buffer
 * 
 * @return True/False as result of request
 */
bool WebClientRequest(WebRequestType type, const char *url, const char *post, char *out);

/**
 * @brief HTTP telegram photo request
 * 
 * @param url Request URL
 * @param chat_id Telegram user ID
 * @param file Path to photo file
 * @param caption Caption under the photo
 * @param out Output buffer
 * 
 * @return True/False as result of request
 */
bool WebClientPhotoRequest(const char *url, unsigned chat_id, const char *file, const char *caption, char *out);

/**
 * @brief HTTP telegram document request
 * 
 * @param url Request URL
 * @param chat_id Telegram user ID
 * @param file Path to photo file
 * @param out Output buffer
 * 
 * @return True/False as result of request
 */
bool WebClientDocumentRequest(const char *url, unsigned chat_id, const char *file, char *out);

#endif /* __WEB_CLIENT_H__ */
