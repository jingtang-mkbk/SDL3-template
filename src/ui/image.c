#include "image.h"
#include "registry.h"

static UI_Image *create(SDL_Renderer *renderer, char *path, SDL_FRect rect)
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

    ui_register((Node *)img); /* 登记到 ui 全局注册表，退出时由 ui.deinit 统一释放 */
    return img;
}

static UI_Image *createWithClick(SDL_Renderer *renderer, char *path, SDL_FRect rect, void *userdata, void *callback)
{
    UI_Image *img = create(renderer, path, rect);
    Node_SetClickWithUserdata((Node *)img, userdata, callback);

    return img;
}

static UI_Image *createWithMultiEvent(SDL_Renderer *renderer, char *path, SDL_FRect rect, void *userdata, Event_Userevent arr[], int count)
{
    UI_Image *img = create(renderer, path, rect);
    Node_SetMultiMouseEvent((Node *)img, userdata, arr, count);

    return img;
}

static void render(SDL_Renderer *renderer, UI_Image *img)
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

static void deinit(UI_Image *img)
{
    if (!img)
        return;

    ui_unregister((Node *)img); /* 先从注册表移除，避免 ui.deinit 二次释放 */
    SDL_free(img);
}

static void setAlpha(UI_Image *img, float alpha)
{
    if (!img)
        return;

    img->node.alpha = SDL_clamp(alpha, 0.0f, 1.0f);
    SDL_SetTextureAlphaMod(img->texture, (Uint8)(alpha * 255));
}

const Image image = {
    .create = create,
    .createWithClick = createWithClick,
    .createWithMultiEvent = createWithMultiEvent,
    .render = render,
    .deinit = deinit,
    .setAlpha = setAlpha,
};