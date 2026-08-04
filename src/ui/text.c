#include "text.h"

static Text *text_arr = NULL;

/*
 * text 必填字段, 计算w、h
 * @param text
 * @param path
 * @param font_size
 * @param color
 */
void text_init(Text *text)
{
  if (!text || !text->text)
    return;

  TTF_Font *font = manager.get_managers()->font_manager->get(text->path ? text->path : "MSYH.TTC");
  if (!font)
    return;

  float saved_size = TTF_GetFontSize(font);
  if (text->font_size > 0.0f)
    TTF_SetFontSize(font, text->font_size);

  int mw;
  size_t ml;
  TTF_MeasureString(font, text->text, 0, 0, &mw, &ml);

  SDL_FRect rect = text->base.rect;
  switch (text->textalign)
  {
  case TextAlign_TopCenter:
    text->base.rect.x = rect.x + (rect.w - mw) / 2.0f;
    break;
  case TextAlign_TopRight:
    text->base.rect.x = rect.x + rect.w - mw;
    break;
  case TextAlign_CenterLeft:
    text->base.rect.y = rect.y + (rect.h - TTF_GetFontHeight(font)) / 2.0f;
    break;
  case TextAlign_Center:
    if (rect.w == 0 || rect.h == 0)
    {
      SDL_Log("请初始化宽高");
      break;
    }
    text->base.rect.x = rect.x + (rect.w - mw) / 2.0f;
    text->base.rect.y = rect.y + (rect.h - TTF_GetFontHeight(font)) / 2.0f;
    break;
  case TextAlign_CenterRight:
    text->base.rect.x = rect.x + rect.w - mw;
    text->base.rect.y = rect.y + (rect.h - TTF_GetFontHeight(font)) / 2.0f;
    break;
  case TextAlign_BottomLeft:
    text->base.rect.y = rect.y + rect.h - TTF_GetFontHeight(font);
    break;
  case TextAlign_BottomCenter:
    text->base.rect.x = rect.x + (rect.w - mw) / 2.0f;
    text->base.rect.y = rect.y + rect.h - TTF_GetFontHeight(font);
    break;
  case TextAlign_BottomRight:
    text->base.rect.x = rect.x + rect.w - mw;
    text->base.rect.y = rect.y + rect.h - TTF_GetFontHeight(font);
    break;
  case TextAlign_TopLeft:
  case TextAlign_None:
  default:
    break;
  }
  text->base.rect.w = (float)mw;
  text->base.rect.h = (float)TTF_GetFontHeight(font);

  TTF_SetFontSize(font, saved_size);
}

void text_render(SDL_Renderer *renderer, Text *text)
{
  if (!text || !text->text)
    return;

  TTF_Font *font = manager.get_managers()->font_manager->get(text->path ? text->path : "MSYH.TTC"); // 默认微软雅黑
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
      /* 只在创建纹理时记录一次，避免 text_arr 无限增长和重复释放 */
      arrput(text_arr, *text);
      SDL_DestroySurface(surf);
    }
  }
  SDL_RenderTexture(renderer, text->texture, NULL, &text->base.rect);
  TTF_SetFontSize(font, saved_size);
}

/*
 * texts 必填字段：
 * @param text
 * @param path
 * @param font_size
 * @param color
 */
void text_init_multiple(Text *texts, int count)
{
  for (int i = 0; i < count; i++)
    text_init(&texts[i]);
}

void text_render_multiple(SDL_Renderer *renderer, Text *texts, int count)
{
  for (int i = 0; i < count; i++)
    text_render(renderer, &texts[i]);
}

void text_deinit()
{
  for (int i = 0; i < arrlen(text_arr); i++)
    SDL_DestroyTexture(text_arr[i].texture);
  arrfree(text_arr);
  text_arr = NULL;
}