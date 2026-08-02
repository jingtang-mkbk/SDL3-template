#ifndef MAIN_SCENE_H
#define MAIN_SCENE_H

#include "data.h"
#include "ui/text.h"
#include "ui/image.h"
#include "render/render.h"
#include "manager/manager.h"

/* Scene-local data — only main_scene.c touches this */
typedef struct MainSceneData
{
  Render_Element *arr;
} MainSceneData;

extern const Scene main_scene;

#endif /* __MAIN_SCENE_H_ */