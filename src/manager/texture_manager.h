#ifndef TEXTURE_MANAGER_H
#define TEXTURE_MANAGER_H

#include "data.h"
#include <SDL3_image/SDL_image.h>

typedef struct TextureManager
{
    void (*init)(void);
    void (*deinit)(void);
    SDL_Texture *(*get)(const char *path);
    SDL_Texture *(*load)(SDL_Renderer *renderer, const char *path);
    void (*remove)(const char *path);
    SDL_Texture *(*load_sprite)(SDL_Renderer *renderer, const char *path, const Uint8 count);
    void (*remove_sprite)(const char *path, const Uint8 count);
} TextureManager;

extern const TextureManager texture_manager;

#endif /* TEXTURE_MANAGER_H */