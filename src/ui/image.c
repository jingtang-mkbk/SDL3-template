#include "image.h"

static Image *img_arr = NULL;

/*
 * img 必填字段, 从 image_manager 获取或加载纹理，计算 w, h
 * @param path
 * @param rect.x
 * @param rect.y
 */
void image_init(SDL_Renderer *renderer, Manager manager, Image *img)
{
  if (!img || !img->path)
    return;

  SDL_Texture *tex = manager.get_managers()->image_manager->get(renderer, img->path);
  if (!tex)
  {
    SDL_Log("image_init: image '%s' not loaded", img->path);
    return;
  }

  if (img->base.rect.w == 0 && img->base.rect.h == 0)
    SDL_GetTextureSize(tex, &img->base.rect.w, &img->base.rect.h);
  img->texture = tex;
}

void image_init_multiple(SDL_Renderer *renderer, Manager manager, Image *imgs, int count)
{
  for (int i = 0; i < count; i++)
    image_init(renderer, manager, &imgs[i]);
}

void image_render(SDL_Renderer *renderer, Image *img)
{
  if (!img || !img->texture)
    return;

  arrput(img_arr, *img);
  SDL_RenderTexture(renderer, img->texture, NULL, &img->base.rect);
}

void image_render_multiple(SDL_Renderer *renderer, Image *imgs, int count)
{
  for (int i = 0; i < count; i++)
    image_render(renderer, &imgs[i]);
}

void image_deinit()
{
  arrfree(img_arr);
  img_arr = NULL;
}