#ifndef TEXT_H
#define TEXT_H

#include <SDL3_ttf/SDL_ttf.h>
#include "data.h"
#include "manager/manager.h"

typedef struct Text
{
  char *text;
  char *path;
  float font_size;
  SDL_Color color;
  SDL_FRect rect;
  SDL_Texture *texture;
} Text;

void text_init(Manager manager, Text *text);
void text_init_multiple(Manager manager, Text *texts, int count);
void text_render(SDL_Renderer *renderer, Manager manager, Text *text);
void text_render_multiple(SDL_Renderer *renderer, Manager manager, Text *texts, int count);
void text_deinit(void);

#endif /* TEXT_H */