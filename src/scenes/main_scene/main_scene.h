#ifndef __MAIN_SCENE_H_
#define __MAIN_SCENE_H_
#include <SDL3/SDL.h>
#include "data.h"
#include "../../engine/image/image.h"

/* Scene-local data — only main_scene.c touches this */
typedef struct MainSceneData
{
  float text_scale;
  float text_x, text_y, text_w, text_h;

  /* Hash map: name → Sprite */
  SpriteEntry *sprites;
} MainSceneData;

extern const Scene main_scene;

#endif /* __MAIN_SCENE_H_ */