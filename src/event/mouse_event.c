#include "mouse_event.h"

static bool check_in_rect(SDL_Renderer *renderer, SDL_Event *event, UI_Event *base)
{
    SDL_ConvertEventToRenderCoordinates(renderer, event);
    float mx = event->button.x;
    float my = event->button.y;

    return hit_in_rect(&base->rect, mx, my);
}

static void mousedown(SDL_Renderer *renderer, SDL_Event *event, UI_Event *base)
{
    if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN && event->button.button == SDL_BUTTON_LEFT) {
        if (!check_in_rect(renderer, event, base))
            return;

        base->event.mousedown(event, base->event.userdata);
    }
}

static void mouseup(SDL_Renderer *renderer, SDL_Event *event, UI_Event *base)
{
    if (event->type == SDL_EVENT_MOUSE_BUTTON_UP && event->button.button == SDL_BUTTON_LEFT) {
        if (!check_in_rect(renderer, event, base))
            return;

        base->event.mouseup(event, base->event.userdata);
    }
}

static void mouseenter(SDL_Renderer *renderer, SDL_Event *event, UI_Event *base)
{
    if (event->type != SDL_EVENT_MOUSE_MOTION)
        return;

    bool inside = check_in_rect(renderer, event, base);

    if (inside && !base->event.mouse_in_rect) {
        base->event.mouseenter(event, base->event.userdata);
        base->event.mouse_in_rect = true; /* 只有边沿进入才置位，避免覆盖 mouseleave 的检测 */
    }
}

static void mouseleave(SDL_Renderer *renderer, SDL_Event *event, UI_Event *base)
{
    if (event->type != SDL_EVENT_MOUSE_MOTION)
        return;

    bool inside = check_in_rect(renderer, event, base);

    if (!inside && base->event.mouse_in_rect) {
        base->event.mouseleave(event, base->event.userdata);
        base->event.mouse_in_rect = false; /* 只有边沿离开才复位，避免覆盖 mouseenter 的检测 */
    }
}

static void mousemove(SDL_Renderer *renderer, SDL_Event *event, UI_Event *base)
{
    if (event->type != SDL_EVENT_MOUSE_MOTION)
        return;

    bool inside = check_in_rect(renderer, event, base);

    if (inside) {
        base->event.mouse_in_rect = true; /* 维护"在内"状态；移出交给 mouseleave 复位 */
        if (base->event.mousemove)
            base->event.mousemove(event, base->event.userdata);
    }
}

static void mousehover(SDL_Renderer *renderer, SDL_Event *event, UI_Event *base)
{
    if (event->type != SDL_EVENT_MOUSE_MOTION)
        return;

    bool inside = check_in_rect(renderer, event, base);
    if (inside) {
        base->event.mousehover(event, base->event.userdata);
    }
}

static void mousedown_right(SDL_Renderer *renderer, SDL_Event *event, UI_Event *base)
{
    if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN && event->button.button == SDL_BUTTON_RIGHT) {
        if (!check_in_rect(renderer, event, base))
            return;

        base->event.mousedown_right(event, base->event.userdata);
    }
}

static void mouseup_right(SDL_Renderer *renderer, SDL_Event *event, UI_Event *base)
{
    if (event->type == SDL_EVENT_MOUSE_BUTTON_UP && event->button.button == SDL_BUTTON_RIGHT) {
        if (!check_in_rect(renderer, event, base))
            return;

        base->event.mouseup_right(event, base->event.userdata);
    }
}

static void click(SDL_Renderer *renderer, SDL_Event *event, UI_Event *base)
{
    if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN && event->button.button == SDL_BUTTON_LEFT) {
        if (check_in_rect(renderer, event, base)) {
            base->event.mousedown_flag = true;
        }
    }
    if (event->type == SDL_EVENT_MOUSE_BUTTON_UP && event->button.button == SDL_BUTTON_LEFT) {
        if (check_in_rect(renderer, event, base)) {
            base->event.click(event, base->event.userdata);
        }
        base->event.mousedown_flag = false;
    }
}

static void click_right(SDL_Renderer *renderer, SDL_Event *event, UI_Event *base)
{
    if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN && event->button.button == SDL_BUTTON_RIGHT) {
        if (check_in_rect(renderer, event, base)) {
            base->event.mousedown_right_flag = true;
        }
    }
    if (event->type == SDL_EVENT_MOUSE_BUTTON_UP && event->button.button == SDL_BUTTON_RIGHT) {
        if (check_in_rect(renderer, event, base)) {
            base->event.click_right(event, base->event.userdata);
        }
        base->event.mousedown_right_flag = false;
    }
}

static void wheel(SDL_Renderer *renderer, SDL_Event *event, UI_Event *base)
{
    if (event->type != SDL_EVENT_MOUSE_WHEEL)
        return;

    SDL_ConvertEventToRenderCoordinates(renderer, event);
    float mx = event->wheel.mouse_x;
    float my = event->wheel.mouse_y;

    if (!hit_in_rect(&base->rect, mx, my)) {
        return;
    }

    if (base->event.wheel)
        base->event.wheel(event, base->event.userdata);
}

static bool checkUserdata(UI_Event *base)
{
    if (!base->event.userdata) {
        SDL_Log("Set userdata before event.\n");
        return false;
    } else
        return true;
}

UI_Event *ui_base_create(void)
{
    return (UI_Event *)SDL_calloc(1, sizeof(UI_Event));
}

void UI_SetPosition(UI_Event *base, float x, float y)
{
    if (!base)
        return;

    base->rect.x = x;
    base->rect.y = y;
}

void UI_SetSize(UI_Event *base, float w, float h)
{
    if (!base)
        return;

    base->rect.w = w;
    base->rect.h = h;
}

void UI_SetAnchor(UI_Event *base, float offsetX, float offsetY)
{
    if (!base)
        return;

    base->anchor.x = base->rect.x + offsetX;
    base->anchor.y = base->rect.y + offsetY;
}

void UI_SetRotate(UI_Event *base, double deg)
{
    if (!base)
        return;

    base->angle = deg;
}

void UI_SetVisible(UI_Event *base, bool visible)
{
    if (!base)
        return;

    base->visible = visible;
}

void UI_SetUserdata(UI_Event *base, void *userdata)
{
    if (!base)
        return;
    base->event.userdata = userdata;
    base->event.event_mask = 0;
    base->event.mouse_in_rect = false;
    base->event.mousedown_flag = false;
    base->event.mousedown_right_flag = false;
}

/* 把某个事件类型注册进 userevent_arr（供 mouseevent() 分发）；已存在则更新回调 */
static void ui_event_register(UI_Event *base, MouseeventType type, void *fn)
{
    for (int i = 0; i < arrlen(base->event.userevent_arr); i++) {
        if (base->event.userevent_arr[i].type == type) {
            base->event.userevent_arr[i].fn = (func_event)fn;
            return;
        }
    }
    Event_Userevent e = { type, (func_event)fn };
    arrput(base->event.userevent_arr, e);
    base->event.userevent_count = (Uint8)arrlen(base->event.userevent_arr);
    base->event.event_mask |= (1u << type);
}

void UI_SetClick(UI_Event *base, void *callback)
{
    if (!base || !checkUserdata(base))
        return;
    base->event.click = callback;
    ui_event_register(base, MOUSEEVENT_CLICK, callback);
}

void UI_SetRightClick(UI_Event *base, void *callback)
{
    if (!base || !checkUserdata(base))
        return;
    base->event.click_right = callback;
    ui_event_register(base, MOUSEEVENT_CLICK_RIGHT, callback);
}

void UI_SetMouseenter(UI_Event *base, void *callback)
{
    if (!base || !checkUserdata(base))
        return;
    base->event.mouseenter = callback;
    ui_event_register(base, MOUSEEVENT_ENTER, callback);
}

void UI_SetMouseleave(UI_Event *base, void *callback)
{
    if (!base || !checkUserdata(base))
        return;
    base->event.mouseleave = callback;
    ui_event_register(base, MOUSEEVENT_LEAVE, callback);
}

void UI_SetClickWithUserdata(UI_Event *base, void *userdata, void *callback)
{
    if (!base)
        return;
    if (!base->event.userdata)
        UI_SetUserdata(base, userdata);

    base->event.click = callback;
    ui_event_register(base, MOUSEEVENT_CLICK, callback);
}

void UI_SetMultiEvent(UI_Event *base, void *userdata, Event_Userevent *arr, Uint8 count)
{
    UI_SetUserdata(base, userdata);
    base->event.userevent_count = count;

    base->event.userevent_arr = NULL;
    for (int i = 0; i < count; i++) {
        arrput(base->event.userevent_arr, arr[i]);
        base->event.event_mask |= (1u << arr[i].type);

        switch (arr[i].type) {
        case MOUSEEVENT_DOWN:
            base->event.mousedown = arr[i].fn;
            break;
        case MOUSEEVENT_UP:
            base->event.mouseup = arr[i].fn;
            break;
        case MOUSEEVENT_ENTER:
            base->event.mouseenter = arr[i].fn;
            break;
        case MOUSEEVENT_LEAVE:
            base->event.mouseleave = arr[i].fn;
            break;
        case MOUSEEVENT_MOVE:
            base->event.mousemove = arr[i].fn;
            break;
        case MOUSEEVENT_HOVER:
            base->event.mousehover = arr[i].fn;
            break;
        case MOUSEEVENT_DOWN_RIGHT:
            base->event.mousedown_right = arr[i].fn;
            break;
        case MOUSEEVENT_UP_RIGHT:
            base->event.mouseup_right = arr[i].fn;
            break;
        case MOUSEEVENT_CLICK:
            base->event.click = arr[i].fn;
            break;
        case MOUSEEVENT_CLICK_RIGHT:
            base->event.click_right = arr[i].fn;
            break;
        case MOUSEEVENT_WHEEL:
            base->event.wheel = arr[i].fn;
            break;
        default:
            break;
        }
    }
}

/* 事件类型 → 分发函数 查表：枚举值即执行顺序（见 mouse_event.h），按下标一一对应 */
static void (*const event_dispatch[])(SDL_Renderer *, SDL_Event *, UI_Event *) = {
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

void mouseevent(SDL_Renderer *renderer, SDL_Event *event, UI_Event *base)
{
    if (!renderer || !base || !event)
        return;

    /* 枚举顺序即执行顺序；event_mask 位掩码 O(1) 判断是否注册，
       查表数组按下标调用对应处理函数 */
    for (size_t i = 0; i < sizeof(event_dispatch) / sizeof(event_dispatch[0]); i++) {
        if (base->event.event_mask & (1u << i))
            event_dispatch[i](renderer, event, base);
    }
}