#include "image.h"

static UI_Image **img_arr = NULL;

UI_Image *UI_Image_Create(SDL_Renderer *renderer, char *path, SDL_FRect rect)
{
    UI_Image *img = (UI_Image *)SDL_calloc(1, sizeof(UI_Image));
    Node_Default((Node *)img, NODETYPE_IMAGE);
    img->node.rect = rect;
    img->path = path;

    SDL_Texture *tex = manager.get_managers()->texture_manager->load(renderer, img->path);
    if (!tex) {
        SDL_Log("UI_Image_Create: image '%s' not loaded", img->path);
        SDL_free(img);
        return NULL;
    }

    if (img->node.rect.w == 0 && img->node.rect.h == 0)
        SDL_GetTextureSize(tex, &img->node.rect.w, &img->node.rect.h);
    img->texture = tex;

    arrput(img_arr, img); /* 注册，deinit 时统一 SDL_free */
    return img;
}

UI_Image *UI_Image_CreateWithClick(SDL_Renderer *renderer, char *path, SDL_FRect rect, void *userdata, void *callback)
{
    UI_Image *img = UI_Image_Create(renderer, path, rect);
    Node_SetClickWithUserdata((Node *)img, userdata, callback);

    return img;
}

UI_Image *UI_Image_CreateWithMultiEvent(SDL_Renderer *renderer, char *path, SDL_FRect rect, void *userdata, Event_Userevent arr[], int count)
{
    UI_Image *img = UI_Image_Create(renderer, path, rect);
    Node_SetMultiMouseEvent((Node *)img, userdata, arr, count);

    return img;
}

void UI_Image_Render(SDL_Renderer *renderer, UI_Image *img)
{
    if (!img)
        return;
    if (!img->texture)
        return;

    /* 与精灵一致：渲染时按 node.alpha 应用透明度 */
    SDL_SetTextureAlphaMod(img->texture, (Uint8)(img->node.alpha * 255));

    if (img->node.angle) {
        SDL_FPoint center = { img->node.rect.w * 0.5f, img->node.rect.h * 0.5f };
        SDL_RenderTextureRotated(renderer, img->texture, NULL, &img->node.rect, img->node.angle,
                                 &center, SDL_FLIP_NONE);
    } else
        SDL_RenderTexture(renderer, img->texture, NULL, &img->node.rect);
}

void UI_Image_Deinit()
{
    /* 释放 UI_Image_Create 中 SDL_calloc 分配的对象 */
    for (int i = 0; i < arrlen(img_arr); i++) {
        arrfree(img_arr[i]->node.event.userevent_arr); /* 释放 arrput 的动态数组 */
        SDL_free(img_arr[i]);
    }
    arrfree(img_arr);
    img_arr = NULL;
}

void UI_Image_SetAlpha(UI_Image *img, float alpha)
{
    if (!img)
        return;

    img->node.alpha = SDL_clamp(alpha, 0.0f, 1.0f);
    SDL_SetTextureAlphaMod(img->texture, (Uint8)(alpha * 255));
}
