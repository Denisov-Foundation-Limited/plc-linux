/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#include <net/web/webclient.h>
#include <utils/utils.h>

#include <curl/curl.h>

#include <string.h>

/*********************************************************************/
/*                                                                   */
/*                         PRIVATE FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

static size_t WebOutputWrite(char *ptr, size_t size, size_t nmemb, void *data)
{
    if (nmemb > BUFFER_LEN_MAX) {
        return 0;
    }

    strncpy((char *)data, ptr, nmemb);

    return size * nmemb;
}

/*********************************************************************/
/*                                                                   */
/*                          PUBLIC FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

bool WebClientRequest(WebRequestType type, const char *url, const char *post, char *out)
{
    CURL    *curl_handle;
    int     ret;

    curl_handle = curl_easy_init();
    if (!curl_handle) {
        return false;
    }

    curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    if (type == WEB_REQ_POST) {
        if (post == NULL) {
            curl_easy_cleanup(curl_handle);
            return false;
        }
        curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, post);
    }

    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, &WebOutputWrite);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, out);
    ret = curl_easy_perform(curl_handle);
    curl_easy_cleanup(curl_handle);

    if (ret != CURLE_OK) {
        return false;
    }

    return true;
}
