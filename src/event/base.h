#ifndef BASE_H
#define BASE_H

#include <SDL3/SDL.h>

typedef void (*FuncEvent)(SDL_Event *event, void *userdata);

typedef enum NodeType
{
    NODETYPE_CONTAINER, /* 容器：不绘制自身，仅用于挂载/分组 children */
    NODETYPE_IMAGE,     /* 图片 */
    NODETYPE_TEXT,      /* 文本 */
    NODETYPE_SPRITE,    /* 精灵 */
} NodeType;

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

typedef struct Event_Userevent
{
    MouseeventType type;
    FuncEvent fn;
} Event_Userevent;

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

typedef struct KeyboardEvent
{
    FuncEvent keydown;
    FuncEvent keyup;
    void *userdata;
} KeyboardEvent;

typedef struct Node Node;
/* Node：控件内嵌基类（几何 rect + 交互 event + 视觉属性） */
typedef struct Node
{
    MouseEvent event;  // 鼠标事件
    KeyboardEvent key; // 键盘事件
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

#endif /* BASE_H */