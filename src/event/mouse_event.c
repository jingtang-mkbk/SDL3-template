#include "mouse_event.h"

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

Node *Node_Create(void)
{
    return (Node *)SDL_calloc(1, sizeof(Node));
}

void Node_Default(Node *node, NodeType type)
{
    node->type = type;
    node->alpha = 1.0f;
    node->anchor = (SDL_FPoint){ 0.0f, 0.0f };
    node->angle = 0.0f;
    node->children = NULL;
    node->visible = true;
    node->z_index = 0;
}

void Node_SetPosition(Node *node, float x, float y)
{
    if (!node)
        return;

    node->rect.x = x;
    node->rect.y = y;
}

void Node_SetSize(Node *node, float w, float h)
{
    if (!node)
        return;

    node->rect.w = w;
    node->rect.h = h;
}

void Node_SetAnchor(Node *node, float offsetX, float offsetY)
{
    if (!node)
        return;

    node->anchor.x = node->rect.x + offsetX;
    node->anchor.y = node->rect.y + offsetY;
}

void Node_SetRotate(Node *node, double deg)
{
    if (!node)
        return;

    node->angle = deg;
}

void Node_SetVisible(Node *node, bool visible)
{
    if (!node)
        return;

    node->visible = visible;
}

void Node_SetUserdata(Node *node, void *userdata)
{
    if (!node)
        return;
    node->event.userdata = userdata;
    node->event.event_mask = 0;
    node->event.mouse_in_rect = false;
    node->event.mousedown_flag = false;
    node->event.mousedown_right_flag = false;
}

/* 把某个事件类型注册进 userevent_arr（供 mouseevent() 分发）；已存在则更新回调 */
static void ui_event_register(Node *node, MouseeventType type, void *fn)
{
    for (int i = 0; i < arrlen(node->event.userevent_arr); i++) {
        if (node->event.userevent_arr[i].type == type) {
            node->event.userevent_arr[i].fn = (func_event)fn;
            return;
        }
    }
    Event_Userevent e = { type, (func_event)fn };
    arrput(node->event.userevent_arr, e);
    node->event.userevent_count = (Uint8)arrlen(node->event.userevent_arr);
    node->event.event_mask |= (1u << type);
}

void Node_SetClick(Node *node, void *callback)
{
    if (!node || !checkUserdata(node))
        return;
    node->event.click = callback;
    ui_event_register(node, MOUSEEVENT_CLICK, callback);
}

void Node_SetRightClick(Node *node, void *callback)
{
    if (!node || !checkUserdata(node))
        return;
    node->event.click_right = callback;
    ui_event_register(node, MOUSEEVENT_CLICK_RIGHT, callback);
}

void Node_SetMouseenter(Node *node, void *callback)
{
    if (!node || !checkUserdata(node))
        return;
    node->event.mouseenter = callback;
    ui_event_register(node, MOUSEEVENT_ENTER, callback);
}

void Node_SetMouseleave(Node *node, void *callback)
{
    if (!node || !checkUserdata(node))
        return;
    node->event.mouseleave = callback;
    ui_event_register(node, MOUSEEVENT_LEAVE, callback);
}

void Node_SetClickWithUserdata(Node *node, void *userdata, void *callback)
{
    if (!node)
        return;
    if (!node->event.userdata)
        Node_SetUserdata(node, userdata);

    node->event.click = callback;
    ui_event_register(node, MOUSEEVENT_CLICK, callback);
}

void Node_SetMultiMouseEvent(Node *node, void *userdata, Event_Userevent *arr, Uint8 count)
{
    Node_SetUserdata(node, userdata);
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

void mouseevent(SDL_Renderer *renderer, SDL_Event *event, Node *node)
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
