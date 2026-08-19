#ifndef MAIN_SCENE_H
#define MAIN_SCENE_H

#include "data.h"
#include "manager/manager.h"
#include "ui/image.h"
#include "ui/text.h"

/* Scene-local data — only main_scene.c touches this */
typedef struct MainSceneData
{
    // Render_Element *arr;
    UI_Text *MineSweeper;
    UI_Text *Test;
    UI_Text *Gomoku;
    UI_Image *Tiger;
} MainSceneData;

extern const Scene main_scene;

#endif /* __MAIN_SCENE_H_ */