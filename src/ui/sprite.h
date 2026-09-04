#ifndef SPRITE_H
#define SPRITE_H

#include "event/mouse_event.h"
#include "manager/manager.h"

typedef struct UI_Sprite
{
    Node node;
    const char *path;
    SDL_Texture *texture;
    Uint8 count;             // 精灵数
    Uint16 ms;               // 每轮播放的毫秒数
    Uint8 index;             // 当前帧索引
    Uint64 last_update_time; // 上次更新时间
} UI_Sprite;

typedef struct Sprite
{
    UI_Sprite *(*create)(SDL_Renderer *renderer, const char *path, SDL_FRect rect, const Uint8 spriteCount, const Uint16 ms);
    UI_Sprite *(*createWithClick)(SDL_Renderer *renderer, const char *path, SDL_FRect rect, void *userdata, void *callback, const Uint8 spriteCount, const Uint16 ms);
    UI_Sprite *(*createWithMultiEvent)(SDL_Renderer *renderer, const char *path, SDL_FRect rect, void *userdata, Event_Userevent arr[], const int count, const Uint8 spriteCount, const Uint16 ms);
    void (*render)(SDL_Renderer *renderer, UI_Sprite *sprite);
    void (*deinit)(UI_Sprite *sprite);
    void (*setAlpha)(UI_Sprite *sprite, float alpha);
} Sprite;

extern const Sprite sprite;

#endif /* SPRITE_H */