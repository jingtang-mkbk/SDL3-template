#include "image.h"

typedef struct UI_Image_internal
{
    Event base;
    char *path;
    bool hidden;
    SDL_Texture *texture;
    float alpha;       // alpha通道
    double angle;      // 旋转角度
    SDL_FPoint anchor; // 锚点
} UI_Image_internal;

static UI_Image_internal **img_arr = NULL;

UI_Image *UI_Image_CreateWithProp(SDL_Renderer *renderer, char *path, SDL_FRect rect)
{
    UI_Image_internal *img = (UI_Image_internal *)SDL_calloc(1, sizeof(UI_Image_internal));
    img->base.rect = rect;
    img->path = path;

    SDL_Texture *tex = manager.get_managers()->image_manager->get(renderer, img->path);
    if (!tex) {
        SDL_Log("UI_Image_CreateWithProp: image '%s' not loaded", img->path);
        return NULL;
    }

    if (img->base.rect.w == 0 && img->base.rect.h == 0)
        SDL_GetTextureSize(tex, &img->base.rect.w, &img->base.rect.h);
    img->texture = tex;

    arrput(img_arr, img); /* 注册，deinit 时统一 SDL_free */

    return (UI_Image *)img;
}

void UI_Image_Render(SDL_Renderer *renderer, UI_Image *_img)
{
    if (!_img)
        return;
    UI_Image_internal *img = (UI_Image_internal *)_img;
    if (!img->texture)
        return;

    if (img->angle) {
        SDL_FPoint center = { img->base.rect.w * 0.5f, img->base.rect.h * 0.5f };
        SDL_RenderTextureRotated(renderer, img->texture, NULL, &img->base.rect, img->angle, &center,
                                 SDL_FLIP_NONE);
    } else
        SDL_RenderTexture(renderer, img->texture, NULL, &img->base.rect);
}

void UI_Image_Deinit()
{
    /* 释放 UI_Image_CreateWithProp 中 SDL_calloc 分配的对象 */
    for (int i = 0; i < arrlen(img_arr); i++)
        SDL_free(img_arr[i]);
    arrfree(img_arr);
    img_arr = NULL;
}

void UI_Image_SetPosition(UI_Image *_img, float x, float y)
{
    if (!_img)
        return;
    UI_Image_internal *img = (UI_Image_internal *)_img;

    img->base.rect.x = x;
    img->base.rect.y = y;
}

void UI_Image_SetSize(UI_Image *_img, float w, float h)
{
    if (!_img)
        return;
    UI_Image_internal *img = (UI_Image_internal *)_img;

    img->base.rect.w = w;
    img->base.rect.h = h;
}

/*
 * \param offsetX 基于矩形的偏移x
 * \param offsetY 基于矩形的偏移y
 */
void UI_Image_SetAnchor(UI_Image *_img, float offsetX, float offsetY)
{
    if (!_img)
        return;
    UI_Image_internal *img = (UI_Image_internal *)_img;

    img->anchor.x = img->base.rect.x + offsetX;
    img->anchor.y = img->base.rect.y + offsetY;
}

void UI_Image_SetAplha(UI_Image *_img, float alpha)
{
    if (!_img)
        return;
    UI_Image_internal *img = (UI_Image_internal *)_img;

    img->alpha = SDL_clamp(alpha, 0.0f, 1.0f);
    SDL_SetTextureAlphaMod(img->texture, (Uint8)(img->alpha * 255));
}

void UI_Image_SetRotate(UI_Image *_img, double deg)
{
    if (!_img)
        return;
    UI_Image_internal *img = (UI_Image_internal *)_img;

    img->angle = deg;
}

void UI_Image_SetHidden(UI_Image *_img, bool hidden)
{
    if (!_img)
        return;
    UI_Image_internal *img = (UI_Image_internal *)_img;

    img->hidden = hidden;
}

void UI_Image_SetUserdata(UI_Image *_img, void *userdata)
{
    if (!_img)
        return;
    UI_Image_internal *img = (UI_Image_internal *)_img;

    img->base.event_userdata = userdata;
}

void UI_Image_SetClick(UI_Image *_img, void *callback)
{
    if (!_img)
        return;
    UI_Image_internal *img = (UI_Image_internal *)_img;
    img->base.click = callback;
}

void UI_Image_SetClickRight(UI_Image *_img, void *callback)
{
    if (!_img)
        return;
    UI_Image_internal *img = (UI_Image_internal *)_img;
    img->base.click_right = callback;
}

void UI_Image_SetMouseenter(UI_Image *_img, void *callback)
{
    if (!_img)
        return;
    UI_Image_internal *img = (UI_Image_internal *)_img;
    img->base.mouseenter = callback;
}

void UI_Image_SetMouseleave(UI_Image *_img, void *callback)
{
    if (!_img)
        return;
    UI_Image_internal *img = (UI_Image_internal *)_img;
    img->base.mouseleave = callback;
}