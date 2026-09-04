#ifndef MAIN_SCENE_H
#define MAIN_SCENE_H

#include "data.h"
#include "manager/manager.h"
#include "ui/ui.h"

/* Scene-local data — only main_scene.c touches this */
typedef struct MainSceneData
{
    // Render_Element *arr;
    Node *root; /* 控件树根（容器），iterate 用 ui.render(renderer, root) 渲染 */
    UI_Text *MineSweeper;
    UI_Text *Test;
    UI_Text *Gomoku;
    UI_Image *Tiger;
    UI_Sprite *SlimeGreen;
    UI_Sprite *SlimeOrange;
} MainSceneData;

extern const Scene main_scene;

#endif /* __MAIN_SCENE_H_ */