/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#include <stdio.h>
#include <string.h>

#include <curl/curl.h>

#include <net/notifier.h>
#include <utils/utils.h>

static struct {
    char        bot[STR_LEN];
    unsigned    chat;
} Telegram = {
    .bot = {0},
    .chat = 0
};

static struct {
    char    api[STR_LEN];
    char    phone[STR_LEN];
} Sms = {
    .api = {0},
    .phone = {0}
};

void NotifierTelegramCredsSet(const char *bot_id, unsigned chat_id)
{
    strncpy(Telegram.bot, bot_id, STR_LEN);
    Telegram.chat = chat_id;
}

void NotifierSmsCredsSet(const char *api, const char *phone)
{
    strncpy(Sms.api, api, STR_LEN);
    strncpy(Sms.phone, phone, STR_LEN);
}

bool NotifierTelegramSend(const char *msg)
{
    char    url[EXT_STR_LEN];
    CURL    *curl_handle;

    snprintf(url, EXT_STR_LEN, "https://api.telegram.org/bot%s/sendMessage?chat_id=%u&text=%s",
            Telegram.bot, Telegram.chat, msg);
    
    curl_handle = curl_easy_init();
    if (curl_handle) {
        curl_easy_setopt(curl_handle, CURLOPT_URL, url);
        curl_easy_perform(curl_handle);
        curl_easy_cleanup(curl_handle);
        return true;
    }

    return false;
}

bool NotifierSmsSend(const char *msg)
{
    char    url[EXT_STR_LEN];
    CURL    *curl_handle;

    snprintf(url, EXT_STR_LEN, "https://sms.ru/sms/send?api_id=%s&to=%s&text=%s&translit=1",
            Sms.api, Sms.phone, msg);
    
    curl_handle = curl_easy_init();
    if (curl_handle) {
        curl_easy_setopt(curl_handle, CURLOPT_URL, url);
        curl_easy_perform(curl_handle);
        curl_easy_cleanup(curl_handle);
        return true;
    }

    return false;
}
