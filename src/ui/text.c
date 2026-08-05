#include "text.h"

typedef struct UI_Text_internal
{
  Event base;
  char *text;
  char *path;
  float font_size;
  SDL_Color color;
  UI_TextAlign textalign;
  SDL_Texture *texture;
} UI_Text_internal;

static UI_Text_internal *text_arr = NULL;

UI_Text *UI_text_create(void)
{
  return (UI_Text *)SDL_calloc(1, sizeof(UI_Text_internal));
}

/*
 * text 必填字段, 计算w、h
 * @param text
 * @param path
 * @param font_size
 * @param color
 */
void UI_text_init(UI_Text *_text)
{
  if (!_text || !_text->text)
    return;

  UI_Text_internal *text = (UI_Text_internal *)_text;
  TTF_Font *font = manager.get_managers()->font_manager->get(text->path ? text->path : DEFAULT_FONT);
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

void UI_text_render(SDL_Renderer *renderer, UI_Text *_text)
{
  if (!_text || !_text->text)
    return;

  UI_Text_internal *text = (UI_Text_internal *)_text;
  TTF_Font *font = manager.get_managers()->font_manager->get(text->path ? text->path : DEFAULT_FONT); // 默认微软雅黑
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

void UI_text_deinit()
{
  for (int i = 0; i < arrlen(text_arr); i++)
    SDL_DestroyTexture(text_arr[i].texture);
  arrfree(text_arr);
  text_arr = NULL;
}