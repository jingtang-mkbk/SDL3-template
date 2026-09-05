#include "mouse_event.h"
#include "node.h"
#include "utils/utils.h"
#include <stdbool.h>

/* —— 树形统一分发（MouseEvent_Build / MouseEvent_Dispatch）状态 —— */
static Node **s_dispatch_nodes = NULL; /* 事件表：后序(子先于父)+兄弟逆序 = 绘制逆序，前部最上层 */
static Node *s_hover_focus = NULL;     /* 当前悬停的最上层节点 */
static Node *s_last_root = NULL;       /* 上次构建事件表的 root */

static bool check_in_rect(SDL_Renderer *renderer, SDL_Event *event, Node *node)
{
    SDL_ConvertEventToRenderCoordinates(renderer, event);
    float mx = event->button.x;
    float my = event->button.y;

    return hit_in_rect(&node->rect, mx, my);
}

static void mousedown(SDL_Renderer *renderer, SDL_Event *event, Node *node)
{
    if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN && event->button.button == SDL_BUTTON_LEFT) {
        if (!check_in_rect(renderer, event, node))
            return;

        node->event.mousedown(event, node->event.userdata);
    }
}

static void mouseup(SDL_Renderer *renderer, SDL_Event *event, Node *node)
{
    if (event->type == SDL_EVENT_MOUSE_BUTTON_UP && event->button.button == SDL_BUTTON_LEFT) {
        if (!check_in_rect(renderer, event, node))
            return;

        node->event.mouseup(event, node->event.userdata);
    }
}

static void mouseenter(SDL_Renderer *renderer, SDL_Event *event, Node *node)
{
    if (event->type != SDL_EVENT_MOUSE_MOTION)
        return;

    bool inside = check_in_rect(renderer, event, node);

    if (inside && !node->event.mouse_in_rect) {
        node->event.mouseenter(event, node->event.userdata);
        node->event.mouse_in_rect = true; /* 只有边沿进入才置位，避免覆盖 mouseleave 的检测 */
    }
}

static void mouseleave(SDL_Renderer *renderer, SDL_Event *event, Node *node)
{
    if (event->type != SDL_EVENT_MOUSE_MOTION)
        return;

    bool inside = check_in_rect(renderer, event, node);

    if (!inside && node->event.mouse_in_rect) {
        node->event.mouseleave(event, node->event.userdata);
        node->event.mouse_in_rect = false; /* 只有边沿离开才复位，避免覆盖 mouseenter 的检测 */
    }
}

static void mousemove(SDL_Renderer *renderer, SDL_Event *event, Node *node)
{
    if (event->type != SDL_EVENT_MOUSE_MOTION)
        return;

    bool inside = check_in_rect(renderer, event, node);

    if (inside) {
        node->event.mouse_in_rect = true; /* 维护"在内"状态；移出交给 mouseleave 复位 */
        if (node->event.mousemove)
            node->event.mousemove(event, node->event.userdata);
    }
}

static void mousehover(SDL_Renderer *renderer, SDL_Event *event, Node *node)
{
    if (event->type != SDL_EVENT_MOUSE_MOTION)
        return;

    bool inside = check_in_rect(renderer, event, node);
    if (inside) {
        node->event.mousehover(event, node->event.userdata);
    }
}

static void mousedown_right(SDL_Renderer *renderer, SDL_Event *event, Node *node)
{
    if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN && event->button.button == SDL_BUTTON_RIGHT) {
        if (!check_in_rect(renderer, event, node))
            return;

        node->event.mousedown_right(event, node->event.userdata);
    }
}

static void mouseup_right(SDL_Renderer *renderer, SDL_Event *event, Node *node)
{
    if (event->type == SDL_EVENT_MOUSE_BUTTON_UP && event->button.button == SDL_BUTTON_RIGHT) {
        if (!check_in_rect(renderer, event, node))
            return;

        node->event.mouseup_right(event, node->event.userdata);
    }
}

static void click(SDL_Renderer *renderer, SDL_Event *event, Node *node)
{
    if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN && event->button.button == SDL_BUTTON_LEFT) {
        if (check_in_rect(renderer, event, node)) {
            node->event.mousedown_flag = true;
        }
    }
    if (event->type == SDL_EVENT_MOUSE_BUTTON_UP && event->button.button == SDL_BUTTON_LEFT) {
        if (check_in_rect(renderer, event, node)) {
            node->event.click(event, node->event.userdata);
        }
        node->event.mousedown_flag = false;
    }
}

static void click_right(SDL_Renderer *renderer, SDL_Event *event, Node *node)
{
    if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN && event->button.button == SDL_BUTTON_RIGHT) {
        if (check_in_rect(renderer, event, node)) {
            node->event.mousedown_right_flag = true;
        }
    }
    if (event->type == SDL_EVENT_MOUSE_BUTTON_UP && event->button.button == SDL_BUTTON_RIGHT) {
        if (check_in_rect(renderer, event, node)) {
            node->event.click_right(event, node->event.userdata);
        }
        node->event.mousedown_right_flag = false;
    }
}

static void wheel(SDL_Renderer *renderer, SDL_Event *event, Node *node)
{
    if (event->type != SDL_EVENT_MOUSE_WHEEL)
        return;

    SDL_ConvertEventToRenderCoordinates(renderer, event);
    float mx = event->wheel.mouse_x;
    float my = event->wheel.mouse_y;

    if (!hit_in_rect(&node->rect, mx, my)) {
        return;
    }

    if (node->event.wheel)
        node->event.wheel(event, node->event.userdata);
}

static bool checkUserdata(Node *node)
{
    if (!node->event.userdata) {
        SDL_Log("Set userdata before event.\n");
        return false;
    } else
        return true;
}

/* Node 的创建/默认/树与几何操作已移入 node.c（见 node.h）；
   此处仅保留事件相关逻辑 */
static void MouseEvent_SetStopPropagation(Node *node, bool stop_propagation)
{
    if (!node)
        return;

    node->event.stop_propagation = stop_propagation;
}

static void MouseEvent_SetUserdata(Node *node, void *userdata)
{
    if (!node)
        return;
    node->event.userdata = userdata;
    node->event.event_mask = 0;
    node->event.mouse_in_rect = false;
    node->event.mousedown_flag = false;
    node->event.mousedown_right_flag = false;
}

/* 把某个事件类型注册进 userevent_arr（供 MouseEvent_Handle 分发）；已存在则更新回调 */
static void event_register(Node *node, MouseeventType type, void *fn)
{
    for (int i = 0; i < arrlen(node->event.userevent_arr); i++) {
        if (node->event.userevent_arr[i].type == type) {
            node->event.userevent_arr[i].fn = (FuncEvent)fn;
            return;
        }
    }
    Event_Userevent e = { type, (FuncEvent)fn };
    arrput(node->event.userevent_arr, e);
    node->event.userevent_count = (Uint8)arrlen(node->event.userevent_arr);
    node->event.event_mask |= (1u << type);
}

static void MouseEvent_SetClick(Node *node, void *callback)
{
    if (!node || !checkUserdata(node))
        return;
    node->event.click = callback;
    event_register(node, MOUSEEVENT_CLICK, callback);
}

static void MouseEvent_SetRightClick(Node *node, void *callback)
{
    if (!node || !checkUserdata(node))
        return;
    node->event.click_right = callback;
    event_register(node, MOUSEEVENT_CLICK_RIGHT, callback);
}

static void MouseEvent_SetMouseenter(Node *node, void *callback)
{
    if (!node || !checkUserdata(node))
        return;
    node->event.mouseenter = callback;
    event_register(node, MOUSEEVENT_ENTER, callback);
}

static void MouseEvent_SetMouseleave(Node *node, void *callback)
{
    if (!node || !checkUserdata(node))
        return;
    node->event.mouseleave = callback;
    event_register(node, MOUSEEVENT_LEAVE, callback);
}

static void MouseEvent_SetClickWithUserdata(Node *node, void *userdata, void *callback)
{
    if (!node)
        return;
    if (!node->event.userdata)
        MouseEvent_SetUserdata(node, userdata);

    node->event.click = callback;
    event_register(node, MOUSEEVENT_CLICK, callback);
}

static void MouseEvent_SetMultiMouseEvent(Node *node, void *userdata, Event_Userevent *arr, Uint8 count)
{
    MouseEvent_SetUserdata(node, userdata);
    node->event.userevent_count = count;

    node->event.userevent_arr = NULL;
    for (int i = 0; i < count; i++) {
        arrput(node->event.userevent_arr, arr[i]);
        node->event.event_mask |= (1u << arr[i].type);

        switch (arr[i].type) {
        case MOUSEEVENT_DOWN:
            node->event.mousedown = arr[i].fn;
            break;
        case MOUSEEVENT_UP:
            node->event.mouseup = arr[i].fn;
            break;
        case MOUSEEVENT_ENTER:
            node->event.mouseenter = arr[i].fn;
            break;
        case MOUSEEVENT_LEAVE:
            node->event.mouseleave = arr[i].fn;
            break;
        case MOUSEEVENT_MOVE:
            node->event.mousemove = arr[i].fn;
            break;
        case MOUSEEVENT_HOVER:
            node->event.mousehover = arr[i].fn;
            break;
        case MOUSEEVENT_DOWN_RIGHT:
            node->event.mousedown_right = arr[i].fn;
            break;
        case MOUSEEVENT_UP_RIGHT:
            node->event.mouseup_right = arr[i].fn;
            break;
        case MOUSEEVENT_CLICK:
            node->event.click = arr[i].fn;
            break;
        case MOUSEEVENT_CLICK_RIGHT:
            node->event.click_right = arr[i].fn;
            break;
        case MOUSEEVENT_WHEEL:
            node->event.wheel = arr[i].fn;
            break;
        default:
            break;
        }
    }
}

/* 事件类型 → 分发函数 查表：枚举值即执行顺序（见 mouse_event.h），按下标一一对应 */
static void (*const event_dispatch[])(SDL_Renderer *, SDL_Event *, Node *) = {
    mouseenter,      /* MOUSEEVENT_ENTER */
    mousemove,       /* MOUSEEVENT_MOVE */
    mousehover,      /* MOUSEEVENT_HOVER */
    wheel,           /* MOUSEEVENT_WHEEL */
    mousedown,       /* MOUSEEVENT_DOWN */
    mouseup,         /* MOUSEEVENT_UP */
    click,           /* MOUSEEVENT_CLICK */
    mousedown_right, /* MOUSEEVENT_DOWN_RIGHT */
    mouseup_right,   /* MOUSEEVENT_UP_RIGHT */
    click_right,     /* MOUSEEVENT_CLICK_RIGHT */
    mouseleave,      /* MOUSEEVENT_LEAVE */
};

static void MouseEvent_Handle(SDL_Renderer *renderer, SDL_Event *event, Node *node)
{
    if (!renderer || !node || !event)
        return;

    /* 枚举顺序即执行顺序；event_mask 位掩码 O(1) 判断是否注册，
       查表数组按下标调用对应处理函数 */
    for (size_t i = 0; i < sizeof(event_dispatch) / sizeof(event_dispatch[0]); i++) {
        if (node->event.event_mask & (1u << i))
            event_dispatch[i](renderer, event, node);
    }
}

/* ==================== 树形统一分发 ==================== */
static bool node_has_events(Node *node)
{
    return node->event.event_mask != 0;
}

/* 后序收集：children 全先于自身（子先于父）；兄弟按 next 逆序（后 append=后绘制=优先级高） */
static void collect_event_nodes(Node *node)
{
    if (!node)
        return;

    Node **kids = NULL;
    for (Node *child = node->children; child; child = child->next)
        arrput(kids, child);
    for (int i = (int)arrlen(kids) - 1; i >= 0; i--)
        collect_event_nodes(kids[i]);
    arrfree(kids);

    if (node->visible && node_has_events(node))
        arrput(s_dispatch_nodes, node);
}

/* 稳定插入排序：z_index 降序（越大越先）；z 相同保持原序（保住“后绘制先触发”） */
static void sort_event_nodes(void)
{
    for (int i = 1; i < arrlen(s_dispatch_nodes); i++) {
        Node *node = s_dispatch_nodes[i];
        int j = i - 1;
        while (j >= 0 && s_dispatch_nodes[j]->z_index < node->z_index) {
            s_dispatch_nodes[j + 1] = s_dispatch_nodes[j];
            j--;
        }
        s_dispatch_nodes[j + 1] = node;
    }
}

/* 命中检测（坐标统一换算到 render 坐标系） */
static bool hit_node(SDL_Renderer *renderer, SDL_Event *event, const SDL_FRect *wr)
{
    SDL_ConvertEventToRenderCoordinates(renderer, event);
    if (event->type == SDL_EVENT_MOUSE_WHEEL)
        return hit_in_rect(wr, event->wheel.mouse_x, event->wheel.mouse_y);
    return hit_in_rect(wr, event->button.x, event->button.y);
}

/* 命中最上层节点（事件表下标小者即最上层） */
static Node *topmost_hit(SDL_Renderer *renderer, SDL_Event *event, Node *root)
{
    for (int i = 0; i < arrlen(s_dispatch_nodes); i++) {
        SDL_FRect wr;
        if (!Node_GetWorldRect(root, s_dispatch_nodes[i], &wr))
            continue;
        if (hit_node(renderer, event, &wr))
            return s_dispatch_nodes[i];
    }
    return NULL;
}

/* 把世界坐标临时写入 node->rect，调用单节点 MouseEvent_Handle 再还原（复用 enter/leave/click 等逻辑） */
static void fire_node(SDL_Renderer *renderer, SDL_Event *event, Node *root, Node *node)
{
    SDL_FRect wr;
    if (!Node_GetWorldRect(root, node, &wr))
        return;

    SDL_FRect saved = node->rect;
    node->rect = wr;
    MouseEvent_Handle(renderer, event, node);
    node->rect = saved;
}

/* 构建/重建 root 的事件表：清空 → 后序收集 → z 稳定排序（初始化/改树后调用一次） */
static void MouseEvent_Build(Node *root)
{
    if (!root)
        return;

    if (root != s_last_root)
        s_hover_focus = NULL; /* 换了场景根，丢弃旧悬停焦点，避免指向已释放节点 */
    s_last_root = root;

    arrfree(s_dispatch_nodes);
    s_dispatch_nodes = NULL;
    collect_event_nodes(root);
    sort_event_nodes();
}

/* 统一分发入口：场景 event() 每帧对其 root 调用一次（内部过滤鼠标事件类型） */
static void MouseEvent_Dispatch(SDL_Renderer *renderer, SDL_Event *event, Node *root)
{
    if (!renderer || !event || !root)
        return;

    /* 只处理鼠标类事件：其余（键盘/退出等）一律忽略 */
    switch (event->type) {
    case SDL_EVENT_MOUSE_MOTION:
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP:
    case SDL_EVENT_MOUSE_WHEEL:
        break;
    default:
        return;
    }

    /* root 变化（切场景）时自动重建；同 root 内的增删子/可见性变更请调用方再 MouseEvent_Build 一次 */
    if (root != s_last_root)
        MouseEvent_Build(root);

    /* 指针类（move/hover/enter/leave/wheel）：只发给命中的最上层节点，不向下传递 */
    if (event->type == SDL_EVENT_MOUSE_MOTION) {
        Node *top = topmost_hit(renderer, event, root);
        if (top != s_hover_focus) {
            if (s_hover_focus) /* 离开旧焦点：触发其 LEAVE */
                fire_node(renderer, event, root, s_hover_focus);
            if (top) /* 进入新焦点：触发 ENTER + MOVE/HOVER */
                fire_node(renderer, event, root, top);
            s_hover_focus = top;
        } else if (top) {
            fire_node(renderer, event, root, top); /* 焦点内持续 MOVE/HOVER */
        }
        return;
    }

    if (event->type == SDL_EVENT_MOUSE_WHEEL) {
        Node *top = topmost_hit(renderer, event, root);
        if (top)
            fire_node(renderer, event, root, top);
        return;
    }

    /* 点按类（DOWN/UP/CLICK 及右键…）：从最上层向下分发，命中即处理；stop_propagation=true 时停止向下 */
    for (int i = 0; i < arrlen(s_dispatch_nodes); i++) {
        Node *n = s_dispatch_nodes[i];
        SDL_FRect wr;
        if (!Node_GetWorldRect(root, n, &wr))
            continue;
        if (!hit_node(renderer, event, &wr))
            continue;
        fire_node(renderer, event, root, n);
        if (n->event.stop_propagation)
            break;
    }
}

const MouseEventApi mouseEvent = {
    .handle = MouseEvent_Handle,
    .build = MouseEvent_Build,
    .dispatch = MouseEvent_Dispatch,
    .setUserdata = MouseEvent_SetUserdata,
    .setClick = MouseEvent_SetClick,
    .setRightClick = MouseEvent_SetRightClick,
    .setMouseenter = MouseEvent_SetMouseenter,
    .setMouseleave = MouseEvent_SetMouseleave,
    .setClickWithUserdata = MouseEvent_SetClickWithUserdata,
    .setMultiMouseEvent = MouseEvent_SetMultiMouseEvent,
    .setStopPropagation = MouseEvent_SetStopPropagation,
};
