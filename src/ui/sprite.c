#include "sprite.h"

static UI_Sprite **sprite_arr = NULL;

UI_Sprite *UI_Sprite_Create(SDL_Renderer *renderer, const char *path, SDL_FRect rect, const Uint8 spriteCount, const Uint16 ms)
{
    UI_Sprite *sprite = (UI_Sprite *)SDL_calloc(1, sizeof(UI_Sprite));
    sprite->base.rect = rect;
    sprite->base.visible = true;
    sprite->path = path;
    sprite->count = spriteCount;
    sprite->ms = ms;
    sprite->index = 0;
    sprite->last_update_time = 0;
    SDL_Texture *tex = manager.get_managers()->texture_manager->load_sprite(renderer, path, spriteCount);
    if (!tex) {
        SDL_Log("UI_Sprite_Create: image '%s' not loaded", sprite->path);
        SDL_free(sprite);
        return NULL;
    }

    if (sprite->base.rect.w == 0 && sprite->base.rect.h == 0)
        SDL_GetTextureSize(tex, &sprite->base.rect.w, &sprite->base.rect.h);
    sprite->texture = tex;
    arrput(sprite_arr, sprite); /* 注册，deinit 时统一 SDL_free */
    return sprite;
}

UI_Sprite *UI_Sprite_CreateWithClick(SDL_Renderer *renderer, const char *path, SDL_FRect rect, void *userdata, void *callback, const Uint8 spriteCount, const Uint16 ms)
{
    UI_Sprite *sprite = UI_Sprite_Create(renderer, path, rect, spriteCount, ms);
    UI_SetClickWithUserdata((UI_Event *)sprite, userdata, callback);

    return sprite;
}

UI_Sprite *UI_Sprite_CreateWithMultiEvent(SDL_Renderer *renderer, const char *path, SDL_FRect rect, void *userdata, Event_Userevent arr[], const int count, const Uint8 spriteCount, const Uint16 ms)
{
    UI_Sprite *sprite = UI_Sprite_Create(renderer, path, rect, spriteCount, ms);
    UI_SetMultiEvent((UI_Event *)sprite, userdata, arr, count);

    return sprite;
}

void UI_Sprite_Render(SDL_Renderer *renderer, UI_Sprite *sprite)
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

    if (sprite->base.angle) {
        SDL_FPoint center = { sprite->base.rect.w * 0.5f, sprite->base.rect.h * 0.5f };
        SDL_RenderTextureRotated(renderer, sprite->texture, NULL, &sprite->base.rect, sprite->base.angle,
                                 &center, SDL_FLIP_NONE);
    } else
        SDL_RenderTexture(renderer, sprite->texture, NULL, &sprite->base.rect);
}

void UI_Sprite_Deinit()
{
    /* 释放 UI_Sprite_Create 中 SDL_calloc 分配的对象 */
    for (int i = 0; i < arrlen(sprite_arr); i++) {
        arrfree(sprite_arr[i]->base.event.userevent_arr);
        SDL_free(sprite_arr[i]);
    }
    arrfree(sprite_arr);
    sprite_arr = NULL;
}

void UI_Sprite_SetAplha(UI_Sprite *sprite, float alpha)
{
    if (!sprite)
        return;
    sprite->base.alpha = alpha;
    SDL_SetTextureAlphaMod(sprite->texture, (Uint8)(alpha * 255));
}