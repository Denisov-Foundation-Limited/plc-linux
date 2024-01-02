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
    int     ret = CURLE_OK;

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

bool WebClientPhotoRequest(const char *url, unsigned chat_id, const char *file, const char *caption, char *out)
{
    CURL                *curl;
    CURLcode            ret = CURLE_OK;
    curl_mime           *mime;
    curl_mimepart       *part;
    struct curl_slist   *headers = NULL;
    char                id_str[STR_LEN];

    snprintf(id_str, STR_LEN, "%u", chat_id);

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl, CURLOPT_POST, 1);
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &WebOutputWrite);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, out);
        
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_slist_append(headers, "Content-Type: multipart/form-data");
        curl_slist_append(headers, "charset=utf-8");
        
        mime = curl_mime_init(curl);
        part = curl_mime_addpart(mime);
        curl_mime_name(part, "chat_id");
        curl_mime_data(part, id_str, CURL_ZERO_TERMINATED);
        part = curl_mime_addpart(mime);
        curl_mime_name(part, "photo");
        curl_mime_filedata(part, file);
        curl_mime_type(part, "image/jpeg");
        part = curl_mime_addpart(mime);
        curl_mime_name(part, "caption");
        curl_mime_data(part, caption, CURL_ZERO_TERMINATED);
        curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);

        ret = curl_easy_perform(curl);
        curl_mime_free(mime);
        curl_slist_free_all(headers);
    }
    curl_easy_cleanup(curl);

    if (ret != CURLE_OK) {
        return false;
    }

    return true;
}

bool WebClientDocumentRequest(const char *url, unsigned chat_id, const char *file, char *out)
{
    CURL                *curl;
    CURLcode            ret = -1;
    curl_mime           *form = NULL;
    curl_mimepart       *field = NULL;
    struct curl_slist*  headerlist = NULL;
    static const char   buf[] = "Expect:";

    curl_global_init(CURL_GLOBAL_ALL);

    curl = curl_easy_init();
    if (curl) {
        form = curl_mime_init(curl);
        field = curl_mime_addpart(form);
        curl_mime_name(field, "document");
        curl_mime_filedata(field, file);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &WebOutputWrite);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, out);
        headerlist = curl_slist_append(headerlist, buf);
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_MIMEPOST, form);
        ret = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        curl_mime_free(form);
        curl_slist_free_all(headerlist);
    }

    if (ret != CURLE_OK) {
        return false;
    }

    return true;
}
