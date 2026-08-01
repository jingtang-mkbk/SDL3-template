#include "image.h"

static Image *img_arr = NULL;

/*
 * img 必填字段: path, rect.x, rect.y
 * 从 image_manager 获取或加载纹理，计算 w, h
 */
void image_init(Manager manager, Image *img)
{
  if (!img || !img->path)
    return;

  SDL_Texture *tex = manager.get_managers()->image_manager->get(img->path);
  if (!tex)
  {
    SDL_Log("image_init: image '%s' not loaded", img->path);
    return;
  }

  if (img->rect.w == 0 && img->rect.h == 0)
    SDL_GetTextureSize(tex, &img->rect.w, &img->rect.h);
  img->texture = tex;
}

void image_init_multiple(Manager manager, Image *imgs, int count)
{
  for (int i = 0; i < count; i++)
    image_init(manager, &imgs[i]);
}

void image_render(SDL_Renderer *renderer, Image *img)
{
  if (!img || !img->texture)
    return;

  arrput(img_arr, *img);
  SDL_RenderTexture(renderer, img->texture, NULL, &img->rect);
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