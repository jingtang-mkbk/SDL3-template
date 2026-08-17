#include "text.h"

typedef struct UI_Text_internal
{
    Event base;
    char *text;
    char *path;
    float font_size;
    SDL_Color color;
    UI_TextAlign textalign; // todo delete
    SDL_Texture *texture;
} UI_Text_internal;

static UI_Text_internal **text_arr = NULL;

/*
 * text 必填字段, 计算w、h
 * @param text
 * @param path
 * @param font_size
 * @param color
 */
UI_Text *UI_Text_CreateWithProp(SDL_FRect rect, char *text, char *path, float fontSize,
                                SDL_Color color, UI_TextAlign textalign)
{
    UI_Text_internal *showText = (UI_Text_internal *)SDL_calloc(1, sizeof(UI_Text_internal));

    if (!text) {
        SDL_Log("UI_Text_CreateWithProp: text is NULL\n");
        SDL_free(showText);
        return NULL;
    }
    showText->base.rect = rect;
    showText->color = color;
    showText->font_size = fontSize;
    showText->path = path;
    showText->text = text;

    TTF_Font *font = manager.get_managers()->font_manager->get(path ? path : DEFAULT_FONT);
    if (!font)
        return NULL;
    float saved_size = TTF_GetFontSize(font);
    if (fontSize > 0.0f)
        TTF_SetFontSize(font, fontSize);

    int mw;
    size_t ml;
    TTF_MeasureString(font, showText->text, 0, 0, &mw, &ml);

    switch (textalign) {
    case TextAlign_TopCenter:
        showText->base.rect.x = rect.x + (rect.w - mw) / 2.0f;
        break;
    case TextAlign_TopRight:
        showText->base.rect.x = rect.x + rect.w - mw;
        break;
    case TextAlign_CenterLeft:
        showText->base.rect.y = rect.y + (rect.h - TTF_GetFontHeight(font)) / 2.0f;
        break;
    case TextAlign_Center:
        if (rect.w == 0 || rect.h == 0) {
            SDL_Log("请初始化宽高");
            break;
        }
        showText->base.rect.x = rect.x + (rect.w - mw) / 2.0f;
        showText->base.rect.y = rect.y + (rect.h - TTF_GetFontHeight(font)) / 2.0f;
        break;
    case TextAlign_CenterRight:
        showText->base.rect.x = rect.x + rect.w - mw;
        showText->base.rect.y = rect.y + (rect.h - TTF_GetFontHeight(font)) / 2.0f;
        break;
    case TextAlign_BottomLeft:
        showText->base.rect.y = rect.y + rect.h - TTF_GetFontHeight(font);
        break;
    case TextAlign_BottomCenter:
        showText->base.rect.x = rect.x + (rect.w - mw) / 2.0f;
        showText->base.rect.y = rect.y + rect.h - TTF_GetFontHeight(font);
        break;
    case TextAlign_BottomRight:
        showText->base.rect.x = rect.x + rect.w - mw;
        showText->base.rect.y = rect.y + rect.h - TTF_GetFontHeight(font);
        break;
    case TextAlign_TopLeft:
    case TextAlign_None:
    default:
        break;
    }
    showText->base.rect.w = (float)mw;
    showText->base.rect.h = (float)TTF_GetFontHeight(font);

    TTF_SetFontSize(font, saved_size);

    arrput(text_arr, showText); /* 注册，deinit 时统一 SDL_free */

    return (UI_Text *)showText;
}

void UI_Text_Render(SDL_Renderer *renderer, UI_Text *_text)
{
    if (!_text || !_text->text)
        return;

    UI_Text_internal *text = (UI_Text_internal *)_text;
    TTF_Font *font = manager.get_managers()->font_manager->get(
        text->path ? text->path : DEFAULT_FONT); // 默认微软雅黑
    if (!font)
        return;

    float saved_size = TTF_GetFontSize(font);
    if (text->font_size > 0.0f)
        TTF_SetFontSize(font, text->font_size);

    if (!text->texture) {
        text->color =
            text->color.a == 0 ? (SDL_Color){ 255, 255, 255, 255 } : text->color; // 默认白色

        SDL_Surface *surf = TTF_RenderText_Blended(font, text->text, 0, text->color);
        if (surf) {
            text->texture = SDL_CreateTextureFromSurface(renderer, surf);
            SDL_DestroySurface(surf);
        }
    }
    SDL_RenderTexture(renderer, text->texture, NULL, &text->base.rect);
    TTF_SetFontSize(font, saved_size);
}

void UI_Text_Deinit()
{
    for (int i = 0; i < arrlen(text_arr); i++) {
        SDL_DestroyTexture(text_arr[i]->texture);
        SDL_free(text_arr[i]); /* 释放 SDL_calloc 分配的对象 */
    }
    arrfree(text_arr);
    text_arr = NULL;
}