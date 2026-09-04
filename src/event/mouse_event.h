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
    NODETYPE_CONTAINER, /* 容器：不绘制自身，仅用于挂载/分组 children */
    NODETYPE_IMAGE,     /* 图片 */
    NODETYPE_TEXT,      /* 文本 */
    NODETYPE_SPRITE,    /* 精灵 */
} NodeType;

typedef void (*FuncEvent)(SDL_Event *event, void *userdata);

typedef struct Event_Userevent
{
    MouseeventType type;
    FuncEvent fn;
} Event_Userevent;

/*
    1. mouseenter -> mousemove/mousehover/wheel -> mousedown -> mouseup -> click -> mouseleave
    2. mouseenter -> mousemove/mousehover/wheel -> mousedown_right -> mouseup_right -> click_right -> mouseleave
    3. mouseenter -> mousemove/mousehover/wheel -> mouseleave
*/

typedef struct MouseEvent
{
    FuncEvent mousedown;
    FuncEvent mouseup;
    FuncEvent mouseenter;
    FuncEvent mouseleave;
    FuncEvent mousemove;
    FuncEvent mousehover;
    FuncEvent mousedown_right;
    FuncEvent mouseup_right;
    FuncEvent click;
    FuncEvent click_right;
    FuncEvent wheel;
    void *userdata;
    Uint32 event_mask;         // 已注册事件类型的位掩码（O(1) 判断，见 UI_SetMultiEvent）
    bool mouse_in_rect;        // 上次鼠标是否在矩形内（hover/leave 边沿检测）
    bool mousedown_flag;       // 是否按下了，用于click
    bool mousedown_right_flag; // 是否按下了，用于click_right
    bool stop_propagation;     // 方案B：点按事件命中处理后，阻止继续向下层节点分发
    // 用户事件
    Event_Userevent *userevent_arr;
    Uint8 userevent_count;
} MouseEvent;

typedef struct Node Node;
/* Node：控件内嵌基类（几何 rect + 交互 event + 视觉属性） */
typedef struct Node
{
    MouseEvent event;  // 鼠标事件
    SDL_FRect rect;    // 渲染矩形
    Uint8 z_index;     // 层级，越大越靠上
    bool visible;      // 是否显示
    float alpha;       // alpha通道
    double angle;      // 旋转角度
    SDL_FPoint anchor; // 锚点
    NodeType type;     // 节点类型
    Node *children;    // 子节点（第一个孩子）
    Node *next;        // 同一父节点下的下一个兄弟
} Node;

Node *Node_Create(void);
void Node_Default(Node *node, NodeType type);
void Node_AddChild(Node *parent, Node *child);

void mouseevent(SDL_Renderer *renderer, SDL_Event *event, Node *node);

/* 构建/重建 root 的事件表（后序收集 + z 全局稳定排序）；树建好后在初始化调用一次即可 */
void mouseevent_init(Node *root);

/* 树形统一分发：按 (z_index 全局降序, 后序/后绘制优先) 命中并派发 root 的整棵子树 */
void mouseevent_dispatch(SDL_Renderer *renderer, SDL_Event *event, Node *root);

void Node_SetPosition(Node *node, float x, float y);
void Node_SetSize(Node *node, float w, float h);
void Node_SetAnchor(Node *node, float offsetX, float offsetY);
void Node_SetRotate(Node *node, double deg);
void Node_SetVisible(Node *node, bool visible);

void Node_SetStopPropagation(Node *node, bool stop_propagation);
void Node_SetUserdata(Node *node, void *userdata);
void Node_SetClick(Node *node, void *callback);
void Node_SetRightClick(Node *node, void *callback);
void Node_SetMouseenter(Node *node, void *callback);
void Node_SetMouseleave(Node *node, void *callback);
void Node_SetClickWithUserdata(Node *node, void *userdata, void *callback);
void Node_SetMultiMouseEvent(Node *node, void *userdata, Event_Userevent *arr, Uint8 count);

#endif // EVENT_H
