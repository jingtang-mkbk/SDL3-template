#include "text.h"

static UI_Text **text_arr = NULL;

UI_Text *UI_Text_Create(SDL_FRect rect, char *txt, char *path, float fontSize, UI_TextAlign textalign)
{
    UI_Text *text = (UI_Text *)SDL_calloc(1, sizeof(UI_Text));

    if (!text) {
        SDL_Log("UI_Text_Create: text is NULL\n");
        return NULL;
    }
    Node_Default((Node *)text, NODETYPE_TEXT);
    text->node.rect = rect;
    text->color = FONT_COLOR_WHITE;
    text->font_size = fontSize;
    text->path = path;
    text->text = txt;

    TTF_Font *font = manager.get_managers()->font_manager->get(path ? path : DEFAULT_FONT_FAMILY);
    if (!font)
        return NULL;
    float saved_size = TTF_GetFontSize(font);
    if (fontSize > 0.0f)
        TTF_SetFontSize(font, fontSize);

    int mw;
    size_t ml;
    TTF_MeasureString(font, text->text, 0, 0, &mw, &ml);

    switch (textalign) {
    case TextAlign_TopCenter:
        text->node.rect.x = rect.x + (rect.w - mw) / 2.0f;
        break;
    case TextAlign_TopRight:
        text->node.rect.x = rect.x + rect.w - mw;
        break;
    case TextAlign_CenterLeft:
        text->node.rect.y = rect.y + (rect.h - TTF_GetFontHeight(font)) / 2.0f;
        break;
    case TextAlign_Center:
        if (rect.w == 0 || rect.h == 0) {
            SDL_Log("请初始化宽高");
            break;
        }
        text->node.rect.x = rect.x + (rect.w - mw) / 2.0f;
        text->node.rect.y = rect.y + (rect.h - TTF_GetFontHeight(font)) / 2.0f;
        break;
    case TextAlign_CenterRight:
        text->node.rect.x = rect.x + rect.w - mw;
        text->node.rect.y = rect.y + (rect.h - TTF_GetFontHeight(font)) / 2.0f;
        break;
    case TextAlign_BottomLeft:
        text->node.rect.y = rect.y + rect.h - TTF_GetFontHeight(font);
        break;
    case TextAlign_BottomCenter:
        text->node.rect.x = rect.x + (rect.w - mw) / 2.0f;
        text->node.rect.y = rect.y + rect.h - TTF_GetFontHeight(font);
        break;
    case TextAlign_BottomRight:
        text->node.rect.x = rect.x + rect.w - mw;
        text->node.rect.y = rect.y + rect.h - TTF_GetFontHeight(font);
        break;
    case TextAlign_TopLeft:
    case TextAlign_None:
    default:
        break;
    }
    text->node.rect.w = (float)mw;
    text->node.rect.h = (float)TTF_GetFontHeight(font);

    TTF_SetFontSize(font, saved_size);

    arrput(text_arr, text); /* 注册，deinit 时统一 SDL_free */

    return text;
}

UI_Text *UI_Text_CreateWithClick(SDL_FRect rect, char *txt, char *path, float fontSize, UI_TextAlign textalign, void *userdata, void *callback)
{
    UI_Text *text = UI_Text_Create(rect, txt, path, fontSize, textalign);
    Node_SetClickWithUserdata((Node *)text, userdata, callback);

    return text;
}

UI_Text *UI_Text_CreateWithMultiEvent(SDL_FRect rect, char *txt, char *path, float fontSize, UI_TextAlign textalign, void *userdata, Event_Userevent arr[], int count)
{
    UI_Text *text = UI_Text_Create(rect, txt, path, fontSize, textalign);
    Node_SetMultiMouseEvent((Node *)text, userdata, arr, count);

    return text;
}

void UI_Text_Render(SDL_Renderer *renderer, UI_Text *text)
{
    if (!text || !text->text)
        return;

    TTF_Font *font = manager.get_managers()->font_manager->get(
        text->path ? text->path : DEFAULT_FONT_FAMILY); // 默认微软雅黑
    if (!font)
        return;

    float saved_size = TTF_GetFontSize(font);
    if (text->font_size > 0.0f)
        TTF_SetFontSize(font, text->font_size);

    if (!text->texture) {
        text->color = text->color.a == 0 ? FONT_COLOR_WHITE : text->color;

        SDL_Surface *surf = TTF_RenderText_Blended(font, text->text, 0, text->color);
        if (surf) {
            text->texture = SDL_CreateTextureFromSurface(renderer, surf);
            SDL_DestroySurface(surf);
        }
    }
    /* texture 为懒创建，SetAplha 可能在创建前调用，渲染时每帧应用才可靠 */
    if (text->texture)
        SDL_SetTextureAlphaMod(text->texture, (Uint8)(text->node.alpha * 255));
    SDL_RenderTexture(renderer, text->texture, NULL, &text->node.rect);
    TTF_SetFontSize(font, saved_size);
}

void UI_Text_Deinit()
{
    for (int i = 0; i < arrlen(text_arr); i++) {
        SDL_DestroyTexture(text_arr[i]->texture);
        arrfree(text_arr[i]->node.event.userevent_arr); /* 释放 arrput 的动态数组 */
        SDL_free(text_arr[i]);                          /* 释放 SDL_calloc 分配的对象 */
    }
    arrfree(text_arr);
    text_arr = NULL;
}

void UI_Text_SetAlpha(UI_Text *text, float alpha)
{
    if (!text)
        return;

    text->node.alpha = SDL_clamp(alpha, 0.0f, 1.0f);
}

void UI_Text_SetColor(UI_Text *text, char *color)
{
    if (!text)
        return;

    text->color = parse_color(color);
}
