#ifndef IMAGE_H
#define IMAGE_H

#include <SDL3_image/SDL_image.h>
#include "element.h"
#include "manager/manager.h"

typedef struct Image
{
  Element base;
  char *path;
  SDL_Texture *texture;
} Image;

void image_init(Manager manager, Image *img);
void image_init_multiple(Manager manager, Image *imgs, int count);
void image_render(SDL_Renderer *renderer, Image *img);
void image_render_multiple(SDL_Renderer *renderer, Image *imgs, int count);
void image_deinit(void);

#endif // IMAGE_H