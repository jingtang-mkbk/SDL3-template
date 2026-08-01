#include "text.h"

static Text *text_arr = NULL;

/*
 * text 必填字段, 计算w、h
 * @param text
 * @param path
 * @param font_size
 * @param color
 */
void text_init(Manager manager, Text *text)
{
  if (!text || !text->text)
    return;

  TTF_Font *font = manager.get_managers()->font_manager->get(text->path ? text->path : "assets/fonts/MSYH.TTC");
  if (!font)
    return;

  float saved_size = TTF_GetFontSize(font);
  if (text->font_size > 0.0f)
    TTF_SetFontSize(font, text->font_size);

  int mw;
  size_t ml;
  TTF_MeasureString(font, text->text, 0, 0, &mw, &ml);
  text->rect.w = (float)mw;
  text->rect.h = (float)TTF_GetFontHeight(font);

  TTF_SetFontSize(font, saved_size);
}

void text_render(SDL_Renderer *renderer, Manager manager, Text *text)
{
  if (!text || !text->text)
    return;

  TTF_Font *font = manager.get_managers()->font_manager->get(text->path ? text->path : "assets/fonts/MSYH.TTC"); // 默认微软雅黑
  if (!font)
    return;

  float saved_size = TTF_GetFontSize(font);
  if (text->font_size > 0.0f)
    TTF_SetFontSize(font, text->font_size);

  if (!text->texture)
  {
    text->color = text->color.a == 0 ? (SDL_Color){255, 255, 255, 255} : text->color; // 默认白色

    SDL_Surface *surf = TTF_RenderText_Blended(font, text->text, 0, text->color);
    if (surf)
    {
      text->texture = SDL_CreateTextureFromSurface(renderer, surf);

      SDL_DestroySurface(surf);
    }
  }
  arrput(text_arr, *text);
  SDL_RenderTexture(renderer, text->texture, NULL, &text->rect);
  TTF_SetFontSize(font, saved_size);
}

/*
 * texts 必填字段：
 * @param text
 * @param path
 * @param font_size
 * @param color
 */
void text_init_multiple(Manager manager, Text *texts, int count)
{
  for (int i = 0; i < count; i++)
    text_init(manager, &texts[i]);
}

void text_render_multiple(SDL_Renderer *renderer, Manager manager, Text *texts, int count)
{
  for (int i = 0; i < count; i++)
    text_render(renderer, manager, &texts[i]);
}

void text_deinit()
{
  for (int i = 0; i < arrlen(text_arr); i++)
    SDL_DestroyTexture(text_arr[i].texture);
  arrfree(text_arr);
  text_arr = NULL;
}