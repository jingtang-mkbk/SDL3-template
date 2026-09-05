#ifndef MOUSE_EVENT_H
#define MOUSE_EVENT_H

#include "node.h"
#include <SDL3/SDL.h>

/* 事件模块 API（实现为静态函数，统一经 mouseEvent 表调用） */
typedef struct MouseEventApi
{
    void (*handle)(SDL_Renderer *renderer, SDL_Event *event, Node *node);   /* 单节点事件分发 */
    void (*build)(Node *root);                                              /* 构建/重建 root 事件表 */
    void (*dispatch)(SDL_Renderer *renderer, SDL_Event *event, Node *root); /* 树形统一分发 */
    void (*setUserdata)(Node *node, void *userdata);
    void (*setClick)(Node *node, void *callback);
    void (*setRightClick)(Node *node, void *callback);
    void (*setMouseenter)(Node *node, void *callback);
    void (*setMouseleave)(Node *node, void *callback);
    void (*setClickWithUserdata)(Node *node, void *userdata, void *callback);
    void (*setMultiMouseEvent)(Node *node, void *userdata, Event_Userevent *arr, Uint8 count);
    void (*setStopPropagation)(Node *node, bool stop_propagation);
} MouseEventApi;

extern const MouseEventApi mouseEvent;

#endif // EVENT_H
