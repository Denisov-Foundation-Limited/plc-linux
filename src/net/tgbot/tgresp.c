/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#include <net/tgbot/tgresp.h>
#include <net/web/webclient.h>
#include <utils/utils.h>

#include <glib-2.0/glib.h>

bool TgRespSend(const char *token, unsigned id, const char *text, json_t *buttons)
{
    char    url[STR_LEN];
    char    buf[BUFFER_LEN_MAX];
    json_t  *root = json_object();
    GString *post = g_string_new("");
    bool    ret = false;

    snprintf(url, STR_LEN, "https://api.telegram.org/bot%s/sendMessage", token);

    json_object_set_new(root, "keyboard", buttons);
    char *markup = json_dumps(root, JSON_INDENT(0));
    g_string_append_printf(post, "chat_id=%d&parse_mode=HTML&text=%s&reply_markup=%s", id, text, markup);
    free(markup);
    json_decref(root);

    memset(buf, 0x0, BUFFER_LEN_MAX);
    ret = WebClientRequest(WEB_REQ_POST, url, post->str, buf);
    g_string_free(post, true);

    return ret;
}

bool TgPhotoRespSend(const char *token, unsigned id, const char *photo, const char *caption)
{
    char    url[STR_LEN];
    char    buf[BUFFER_LEN_MAX];

    snprintf(url, STR_LEN, "https://api.telegram.org/bot%s/sendPhoto", token);

    memset(buf, 0x0, BUFFER_LEN_MAX);

    return WebClientPhotoRequest(url, id, photo, caption, buf);
}

void TgRespButtonAdd(json_t *buttons, const char *name)
{
    json_t *butline = json_array();
    json_array_append_new(butline, json_string(name));
    json_array_append_new(buttons, butline);
}

void TgRespButtonsAdd(json_t *buttons, unsigned count, const char *name[])
{
    json_t *butline = json_array();

    for (unsigned i = 0; i < count; i++) {
        json_array_append_new(butline, json_string(name[i]));
    }

    json_array_append_new(buttons, butline);
}
