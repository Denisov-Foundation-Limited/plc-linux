/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#include <cam/camera.h>

#include <stdio.h>

/*********************************************************************/
/*                                                                   */
/*                         PRIVATE VARIABLES                         */
/*                                                                   */
/*********************************************************************/

static struct _Cameras {
    GList   *cameras;
    char    path[STR_LEN];
} Cameras = {
    .cameras = NULL,
};

/*********************************************************************/
/*                                                                   */
/*                          PUBLIC FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

void CameraPathSet(const char *path)
{
    strncpy(Cameras.path, path, STR_LEN);
}

char *CameraPathGet()
{
    return (char *)Cameras.path;
}

Camera *CameraGet(const char *name)
{
    for (GList *s = Cameras.cameras; s != NULL; s = s->next) {
        Camera *cam = (Camera *)s->data;
        if (!strcmp(cam->name, name)) {
            return cam;
        }
    }
    return NULL;
}

bool CameraPhotoSave(Camera *cam, const char *filename)
{
    char    cmd[STR_LEN];
    char    full_path[STR_LEN];
    
    snprintf(full_path, STR_LEN, "%s%s", Cameras.path, filename);

    if (cam->type == CAM_TYPE_IP) {
        snprintf(cmd, STR_LEN,
            "ffmpeg -rtsp_transport tcp -i \"rtsp://%s:554/stream%u?username=%s&password=%s\" -frames:v 1 -q:v 1 -y %s > /dev/null",
            cam->ipcam.ip,
            cam->ipcam.stream,
            cam->ipcam.login,
            cam->ipcam.password,
            full_path
        );
        if (system(cmd) == 0) {
            return true;
        }
    }
    return false;
}

Camera *CameraNew(const char *name, CameraType type)
{
    Camera *cam = (Camera *)malloc(sizeof(Camera));

    strncpy(cam->name, name, SHORT_STR_LEN);
    cam->type = type;

    return cam;
}

GList **CamerasGet()
{
    return &Cameras.cameras;
}

bool CameraAdd(const Camera *cam)
{
    Cameras.cameras = g_list_append(Cameras.cameras, (void *)cam);
    return true;
}
