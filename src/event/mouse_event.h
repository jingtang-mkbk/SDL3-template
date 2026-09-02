#ifndef MOUSE_EVENT_H
#define MOUSE_EVENT_H

#include "data.h"
#include "utils/utils.h"
#include <stdbool.h>

// #define EVENT_COUNT 16

/* 双层展开：## 会抑制操作数展开，需先经一层宏把 EVENT_COUNT 展开成数值，
   再拼接出 Uint16 / Uint32 / Uint64（EVENT_COUNT 改为 32 或 64 时类型自动跟随） */
// #define MOUSE_CONCAT_(a, b) a##b
// #define MOUSE_CONCAT(a, b)  MOUSE_CONCAT_(a, b)
// #define EventFlag           MOUSE_CONCAT(Uint, EVENT_COUNT)

typedef enum MouseeventType
{
    MOUSEEVENT_ENTER,       /* 进入 */
    MOUSEEVENT_MOVE,        /* 移动 */
    MOUSEEVENT_HOVER,       /* 悬停 */
    MOUSEEVENT_WHEEL,       /* 滚轮 */
    MOUSEEVENT_DOWN,        /* 左键按下 */
    MOUSEEVENT_UP,          /* 左键抬起 */
    MOUSEEVENT_CLICK,       /* 左键点击 */
    MOUSEEVENT_DOWN_RIGHT,  /* 右键按下 */
    MOUSEEVENT_UP_RIGHT,    /* 右键抬起 */
    MOUSEEVENT_CLICK_RIGHT, /* 右键点击 */
    MOUSEEVENT_LEAVE,       /* 离开 */
} MouseeventType;

typedef enum NodeType
{
    NODETYPE_IMAGE,  /* 图片 */
    NODETYPE_TEXT,   /* 文本 */
    NODETYPE_SPRITE, /* 精灵 */
} NodeType;

typedef void (*func_event)(SDL_Event *event, void *userdata);

typedef struct Event_Userevent
{
    MouseeventType type;
    func_event fn;
} Event_Userevent;

/*
    1. mouseenter -> mousemove/mousehover/wheel -> mousedown -> mouseup -> click -> mouseleave
    2. mouseenter -> mousemove/mousehover/wheel -> mousedown_right -> mouseup_right -> click_right -> mouseleave
    3. mouseenter -> mousemove/mousehover/wheel -> mouseleave
*/

typedef struct MouseEvent
{
    void (*mousedown)(SDL_Event *event, void *userdata);
    void (*mouseup)(SDL_Event *event, void *userdata);
    void (*mouseenter)(SDL_Event *event, void *userdata);
    void (*mouseleave)(SDL_Event *event, void *userdata);
    void (*mousemove)(SDL_Event *event, void *userdata);
    void (*mousehover)(SDL_Event *event, void *userdata);
    void (*mousedown_right)(SDL_Event *event, void *userdata);
    void (*mouseup_right)(SDL_Event *event, void *userdata);
    void (*click)(SDL_Event *event, void *userdata);
    void (*click_right)(SDL_Event *event, void *userdata);
    void (*wheel)(SDL_Event *event, void *userdata);
    void *userdata;
    Uint32 event_mask;         // 已注册事件类型的位掩码（O(1) 判断，见 UI_SetMultiEvent）
    bool mouse_in_rect;        // 上次鼠标是否在矩形内（hover/leave 边沿检测）
    bool mousedown_flag;       // 是否按下了，用于click
    bool mousedown_right_flag; // 是否按下了，用于click_right
    // 用户事件
    Event_Userevent *userevent_arr;
    Uint8 userevent_count;
} MouseEvent;

typedef struct Node Node;
/* Node：控件内嵌基类（几何 rect + 交互 event + 视觉属性） */
typedef struct Node
{
    MouseEvent event;
    SDL_FRect rect;
    Uint8 z_index;     // 层级，越大越靠上
    bool visible;      // 是否显示
    float alpha;       // alpha通道
    double angle;      // 旋转角度
    SDL_FPoint anchor; // 锚点
    NodeType type;     // 节点类型
    Node *children;
} Node;

Node *Node_Create(void);
void Node_Default(Node *node, NodeType type);

void mouseevent(SDL_Renderer *renderer, SDL_Event *event, Node *node);

void Node_SetPosition(Node *node, float x, float y);
void Node_SetSize(Node *node, float w, float h);
void Node_SetAnchor(Node *node, float offsetX, float offsetY);
void Node_SetRotate(Node *node, double deg);
void Node_SetVisible(Node *node, bool visible);

void Node_SetUserdata(Node *node, void *userdata);
void Node_SetClick(Node *node, void *callback);
void Node_SetRightClick(Node *node, void *callback);
void Node_SetMouseenter(Node *node, void *callback);
void Node_SetMouseleave(Node *node, void *callback);
void Node_SetClickWithUserdata(Node *node, void *userdata, void *callback);
void Node_SetMultiMouseEvent(Node *node, void *userdata, Event_Userevent *arr, Uint8 count);

#endif // EVENT_H
