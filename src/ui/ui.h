#ifndef UI_H
#define UI_H

#include "image.h"
#include "sprite.h"
#include "text.h"

typedef struct Components
{
    const Image *image;
    const Sprite *sprite;
    const Text *text;
} Components;

typedef struct UI
{
    void (*init)(void);
    void (*deinit)(void);
    void (*render)(SDL_Renderer *renderer, Node *root);
    void (*release_root)(Node *root); /* 释放 root 及其下所有组件：注销注册表、销毁 text 贴图、image/sprite 移除贴图缓存（切回自动重载） */
    const Components *(*getComps)(void);
} UI;

extern const UI ui;

#endif /* UI_H */