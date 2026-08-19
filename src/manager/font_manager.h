#ifndef FONT_MANAGER_H
#define FONT_MANAGER_H

#include "data.h"
#include <SDL3_ttf/SDL_ttf.h>

#define DEFAULT_FONT_PATH   "assets/fonts/"
#define DEFAULT_FONT_SIZE   12
#define FONT_COLOR_WHITE    (SDL_Color){ 255, 255, 255, 255 }
#define FONT_COLOR_BLACK    (SDL_Color){ 255, 255, 255, 255 }
#define DEFAULT_FONT_FAMILY "MSYH.TTC"

typedef struct FontManager
{
    bool (*init)(void);
    void (*deinit)(void);
    TTF_Font *(*get)(const char *font_path);
} FontManager;

extern const FontManager font_manager;

#endif /* FONT_MANAGER_H */