#ifndef IMAGE_MANAGER_H
#define IMAGE_MANAGER_H

#include "data.h"
#include <SDL3_image/SDL_image.h>

#define IMAGE_BASE_PATH "assets/imgs/"

typedef struct ImageManager
{
    void (*init)(void);
    void (*deinit)(void);
    SDL_Texture *(*get)(SDL_Renderer *renderer, const char *image_path);
} ImageManager;

extern const ImageManager image_manager;

#endif /* IMAGE_MANAGER_H */