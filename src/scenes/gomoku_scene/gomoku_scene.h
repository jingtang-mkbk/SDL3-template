#ifndef GOMOKU_SCENE_H
#define GOMOKU_SCENE_H

#include "data.h"
#include "manager/manager.h"
#include "ui/image.h"

typedef struct GomokuSceneData
{
    UI_Image *background_img;
    UI_Image *whiteArr;
    UI_Image *blackArr;
} GomokuSceneData;

extern const Scene gomoku_scene;

#endif