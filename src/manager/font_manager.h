#ifndef FONT_MANAGER_H
#define FONT_MANAGER_H

#include <SDL3_ttf/SDL_ttf.h>
#include "data.h"

typedef struct FontEntry
{
  char *key;
  TTF_Font *value;
} FontEntry;

typedef struct FontManager
{
  bool (*init)(void);
  void (*deinit)(void);
  void (*load)(const char *font_path);
  void (*load_multiple)(const char **font_paths, int count);
  TTF_Font *(*get)(const char *font_path);
} FontManager;

extern const FontManager font_manager;

#endif /* FONT_MANAGER_H */