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

#include <net/tgbot/handlers/tgcam.h>
#include <net/tgbot/tgresp.h>
#include <stack/rpc.h>
#include <utils/log.h>

/*********************************************************************/
/*                                                                   */
/*                          PUBLIC FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

void TgCamProcess(const char *token, unsigned from, const char *message)
{
    json_t      *buttons = json_array();
    GList       *cams = NULL;
    GString     *text = g_string_new("<b>КАМЕРЫ</b>\n\n");
    char        file_name[SHORT_STR_LEN];
    char        file_full_name[SHORT_STR_LEN];
    char        cam_path[STR_LEN];

    if (RpcCamerasGet(RPC_DEFAULT_UNIT, &cams)) {
        for (GList *c = cams; c != NULL; c = c->next) {
            RpcCamera *cam = (RpcCamera *)c->data;

            TgRespButtonAdd(buttons, cam->name);

            if (!strcmp(message, cam->name)) {
                if (RpcCameraPathGet(RPC_DEFAULT_UNIT, cam_path)) {

                    snprintf(file_name, SHORT_STR_LEN, "%u.jpg", from);
                    snprintf(file_full_name, SHORT_STR_LEN, "%s%u.jpg", cam_path, from);

                    if (RpcCameraPhotoSave(RPC_DEFAULT_UNIT, cam->name, file_name)) {
                        if (!TgPhotoRespSend(token, from, file_full_name, cam->name)) {
                            text = g_string_append(text, "<b>Ошибка отправки фото</b>");
                            LogF(LOG_TYPE_ERROR, "TGCAM", "Failed to send cam photo for user %d", from);
                        }
                    } else {
                        text = g_string_append(text, "<b>Ошибка получения фото</b>");
                        LogF(LOG_TYPE_ERROR, "TGCAM", "Failed to save photo for user %d", from);
                    }
                } else {
                    text = g_string_append(text, "<b>Ошибка получения папки с фото</b>");
                    LogF(LOG_TYPE_ERROR, "TGCAM", "Failed to save photo folder for user %d", from);
                }
            }

            free(cam);
        }
        g_list_free(cams);
    } else {
        text = g_string_append(text, "<b>Ошибка получения списка камер</b>");
        LogF(LOG_TYPE_ERROR, "TGCAM", "Failed to get cam list for user %d", from);
    }

    TgRespButtonAdd(buttons, "Назад");
    TgRespSend(token, from, text->str, buttons);
    g_string_free(text, true);
}
