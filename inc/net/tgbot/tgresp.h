/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#ifndef __TG_RESP_H__
#define __TG_RESP_H__

#include <stdbool.h>
#include <jansson.h>

bool TgRespSend(const char *token, unsigned id, const char *text, json_t *buttons);

void TgRespButtonAdd(json_t *buttons, const char *name);

void TgRespButtonsAdd(json_t *buttons, unsigned count, const char *name[]);

bool TgPhotoRespSend(const char *token, unsigned id, const char *photo, const char *caption);

#endif /* __TG_RESP_H__ */
