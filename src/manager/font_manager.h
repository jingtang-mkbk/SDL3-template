#ifndef FONT_MANAGER_H
#define FONT_MANAGER_H

#include "data.h"
#include <SDL3_ttf/SDL_ttf.h>

#define FONT_BASE_PATH "assets/fonts/"

typedef struct FontManager
{
    bool (*init)(void);
    void (*deinit)(void);
    TTF_Font *(*get)(const char *font_path);
} FontManager;

extern const FontManager font_manager;

#endif /* FONT_MANAGER_H */