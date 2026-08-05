#include "image.h"

typedef struct UI_Image_internal
{
  Event base;
  char *path;
  SDL_Texture *texture;
} UI_Image_internal;

static UI_Image_internal *img_arr = NULL;

/* 按完整内部结构体分配，返回公开基类指针 */
UI_Image *UI_image_create(void)
{
  return (UI_Image *)SDL_calloc(1, sizeof(UI_Image_internal));
}

/*
 * img 必填字段, 从 image_manager 获取或加载纹理，计算 w, h
 * @param path
 * @param rect.x
 * @param rect.y
 */
void UI_image_init(SDL_Renderer *renderer, Manager manager, UI_Image *_img)
{
  if (!_img || !_img->path)
    return;
  UI_Image_internal *img = (UI_Image_internal *)_img;

  SDL_Texture *tex = manager.get_managers()->image_manager->get(renderer, img->path);
  if (!tex)
  {
    SDL_Log("UI_image_init: image '%s' not loaded", img->path);
    return;
  }

  if (img->base.rect.w == 0 && img->base.rect.h == 0)
    SDL_GetTextureSize(tex, &img->base.rect.w, &img->base.rect.h);
  img->texture = tex;
}

void UI_image_render(SDL_Renderer *renderer, UI_Image *_img)
{
  if (!_img)
    return;
  UI_Image_internal *img = (UI_Image_internal *)_img;
  if (!img->texture)
    return;

  arrput(img_arr, *img);
  SDL_RenderTexture(renderer, img->texture, NULL, &img->base.rect);
}

void UI_image_deinit()
{
  arrfree(img_arr);
  img_arr = NULL;
}