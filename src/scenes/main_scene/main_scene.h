#ifndef MAIN_SCENE_H
#define MAIN_SCENE_H

#include "data.h"
#include "ui/text.h"
#include "ui/image.h"

/* Scene-local data — only main_scene.c touches this */
typedef struct MainSceneData
{
  Text switch_tex;
  Image tiger_img;
} MainSceneData;

extern const Scene main_scene;

#endif /* __MAIN_SCENE_H_ */