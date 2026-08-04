#ifndef IMAGE_MANAGER_H
#define IMAGE_MANAGER_H

#include <SDL3_image/SDL_image.h>
#include "data.h"

#define IMAGE_BASE_PATH "assets/imgs/"

typedef struct ImageEntry
{
  char *key;
  SDL_Texture *value;
} ImageEntry;

typedef struct ImageManager
{
  void (*init)(void);
  void (*deinit)(void);
  SDL_Texture *(*get)(SDL_Renderer *renderer, const char *image_path);
} ImageManager;

extern const ImageManager image_manager;

#endif /* IMAGE_MANAGER_H */