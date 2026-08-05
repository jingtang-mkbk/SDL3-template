#ifndef IMAGE_H
#define IMAGE_H

#include <SDL3_image/SDL_image.h>
#include "event/event.h"
#include "manager/manager.h"

typedef struct UI_Image
{
  Event base;
  char *path;
} UI_Image;

void UI_image_init(SDL_Renderer *renderer, Manager manager, UI_Image *img);
void UI_image_render(SDL_Renderer *renderer, UI_Image *img);
void UI_image_deinit(void);
UI_Image *UI_image_create(void);

#endif // IMAGE_H