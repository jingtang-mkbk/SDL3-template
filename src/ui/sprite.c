#include "sprite.h"
#include "registry.h"

static UI_Sprite *create(SDL_Renderer *renderer, const char *path, SDL_FRect rect, const Uint8 spriteCount, const Uint16 ms)
{
    UI_Sprite *sprite = (UI_Sprite *)SDL_calloc(1, sizeof(UI_Sprite));
    Node_Default((Node *)sprite, NODETYPE_SPRITE);
    sprite->node.rect = rect;
    sprite->path = path;
    sprite->count = spriteCount;
    sprite->ms = ms;
    sprite->index = 0;
    sprite->last_update_time = 0;
    SDL_Texture *tex = manager.get_managers()->texture_manager->load_sprite(renderer, path, spriteCount, NULL);
    if (!tex) {
        SDL_Log("UI_Sprite_Create: image '%s' not loaded", sprite->path);
        SDL_free(sprite);
        return NULL;
    }

    if (sprite->node.rect.w == 0 && sprite->node.rect.h == 0)
        SDL_GetTextureSize(tex, &sprite->node.rect.w, &sprite->node.rect.h);
    sprite->texture = tex;
    ui_register((Node *)sprite); /* 登记到 ui 全局注册表，退出时由 ui.deinit 统一释放 */
    return sprite;
}

static UI_Sprite *createWithClick(SDL_Renderer *renderer, const char *path, SDL_FRect rect, void *userdata, void *callback, const Uint8 spriteCount, const Uint16 ms)
{
    UI_Sprite *sprite = create(renderer, path, rect, spriteCount, ms);
    Node_SetClickWithUserdata((Node *)sprite, userdata, callback);

    return sprite;
}

static UI_Sprite *createWithMultiEvent(SDL_Renderer *renderer, const char *path, SDL_FRect rect, void *userdata, Event_Userevent arr[], const int count, const Uint8 spriteCount, const Uint16 ms)
{
    UI_Sprite *sprite = create(renderer, path, rect, spriteCount, ms);
    Node_SetMultiMouseEvent((Node *)sprite, userdata, arr, count);

    return sprite;
}

static void render(SDL_Renderer *renderer, UI_Sprite *sprite)
{
    if (!sprite)
        return;
    if (!sprite->texture)
        return;

    Uint64 gap_time = sprite->ms / sprite->count;
    Uint64 current_time = SDL_GetTicks();
    if (current_time - sprite->last_update_time >= gap_time) {
        sprite->index = (sprite->index + 1) % sprite->count;
        sprite->last_update_time = current_time;

        char full[100];
        SDL_snprintf(full, sizeof(full), "%s%03d", sprite->path, sprite->index);
        SDL_Texture *next = manager.get_managers()->texture_manager->get(full);
        if (next) /* get 失败时保留当前帧，避免 texture 变 NULL 导致永久不渲染 */
            sprite->texture = next;
    }

    /* 精灵换帧会替换 texture，alpha 必须每帧重新应用到当前贴图才生效 */
    SDL_SetTextureAlphaMod(sprite->texture, (Uint8)(sprite->node.alpha * 255));

    if (sprite->node.angle) {
        SDL_FPoint center = { sprite->node.rect.w * 0.5f, sprite->node.rect.h * 0.5f };
        SDL_RenderTextureRotated(renderer, sprite->texture, NULL, &sprite->node.rect, sprite->node.angle,
                                 &center, SDL_FLIP_NONE);
    } else
        SDL_RenderTexture(renderer, sprite->texture, NULL, &sprite->node.rect);
}

static void deinit(UI_Sprite *sprite)
{
    if (!sprite)
        return;

    ui_unregister((Node *)sprite); /* 先从注册表移除，避免 ui.deinit 二次释放 */
    SDL_free(sprite);
}

static void setAlpha(UI_Sprite *sprite, float alpha)
{
    if (!sprite)
        return;
    sprite->node.alpha = alpha;
}

const Sprite sprite = {
    .create = create,
    .createWithClick = createWithClick,
    .createWithMultiEvent = createWithMultiEvent,
    .render = render,
    .deinit = deinit,
    .setAlpha = setAlpha,
};