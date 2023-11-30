/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#ifndef __CAM_H__
#define __CAM_H__

#include <stdbool.h>

#include <glib-2.0/glib.h>

#include <utils/utils.h>

typedef enum {
    CAM_TYPE_USB,
    CAM_TYPE_IP
} CameraType;

typedef struct {
    unsigned    id;
} CameraUSB;

typedef struct {
    char        ip[SHORT_STR_LEN];
    unsigned    stream;
    char        login[SHORT_STR_LEN];
    char        password[SHORT_STR_LEN];
} CameraIP;

typedef struct {
    char        name[SHORT_STR_LEN];
    CameraType  type;
    CameraIP    ipcam;
    CameraUSB   usbcam;
} Camera;

/**
 * @brief Make new Camera object
 * 
 * @param name Name of Camera
 * @param type Type of camera
 * 
 * @return LCD object
 */
Camera *CameraNew(const char *name, CameraType type);

/**
 * @brief Get all Camera modules
 *
 * @return Cameras
 */
GList **CamerasGet();

/**
 * @brief Get current path for save photos
 *
 * @return path
 **/
char *CameraPathGet();

/**
 * @brief Set new path for save photos
 *
 * @param path Folder
 */
void CameraPathSet(const char *path);

/**
 * @brief Get camera by name
 *
 * @param name Name of camera
 *
 * @return Camera object
 */
Camera *CameraGet(const char *name);

/**
 * @brief Save photo from camera
 *
 * @param cam Camera object
 * @param filename Photo name
 *
 * @return True/False as result of saving
 */
bool CameraPhotoSave(Camera *cam, const char *filename);

/**
 * @brief Add new Camera module
 * 
 * @param cam Camera module object
 * 
 * @return true/false as result of initialization of module
 */
bool CameraAdd(const Camera *cam);

#endif /* __CAM_H__ */
