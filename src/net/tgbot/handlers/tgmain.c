/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#include <jansson.h>

#include <net/tgbot/handlers/tgmain.h>
#include <net/tgbot/tgresp.h>

/*********************************************************************/
/*                                                                   */
/*                          PUBLIC FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

void TgMainProcess(const char *token, unsigned from, const char *message)
{
    json_t      *buttons = json_array();
    const char  *line0[] = { "Я дома", "Ушёл" };
    const char  *line1[] = { "Метео", "Охрана", "Розетки" };
    const char  *line2[] = { "Термо", "Бак", "Полив" };

    TgRespButtonsAdd(buttons, 2, line0);
    TgRespButtonsAdd(buttons, 3, line1);
    TgRespButtonsAdd(buttons, 3, line2);
    TgRespSend(token, from, "<b>ГЛАВНОЕ МЕНЮ</b>\n", buttons);
}
