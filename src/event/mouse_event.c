#include "mouse_event.h"

/* —— 树形事件分发（mouseevent_dispatch）状态 —— */
static Node **s_event_nodes = NULL; /* 事件表：后序(子先于父)+兄弟逆序 = 绘制逆序，下标小者最上层 */
static Node *s_hover_focus = NULL;  /* 当前悬停的最上层节点 */
static Node *s_last_root = NULL;    /* 上次构建事件表的 root */
static bool s_dirty = false;        /* 树结构/可见性变化后置位，下次 dispatch 前重建事件表 */

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
    node->next = NULL;
    node->visible = true;
    node->z_index = 0;
}

/* 把 child 追加到 parent 的孩子链表尾部（保持插入顺序） */
void Node_AddChild(Node *parent, Node *child)
{
    if (!parent || !child)
        return;

    if (!parent->children) {
        parent->children = child;
        return;
    }
    Node *tail = parent->children;
    while (tail->next)
        tail = tail->next;
    tail->next = child;

    s_dirty = true; /* 树结构变化：事件表下次重建 */
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

    if (node->visible != visible) {
        node->visible = visible;
        s_dirty = true; /* 事件表只收录可见节点，可见性变化需重建 */
    }
}

void Node_SetStopPropagation(Node *node, bool stop_propagation)
{
    if (!node)
        return;

    node->event.stop_propagation = stop_propagation;
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
            node->event.userevent_arr[i].fn = (FuncEvent)fn;
            return;
        }
    }
    Event_Userevent e = { type, (FuncEvent)fn };
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

/* ==================== 树形统一分发 ==================== */
static bool node_has_events(Node *node)
{
    return node->event.event_mask != 0;
}

/* 后序遍历收集：children 全部先于自身（子先于父）；兄弟按 next 逆序（后 append=后绘制=优先级高） */
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
        arrput(s_event_nodes, node);
}

/* 稳定插入排序：z_index 降序（越大越先）；z 相同保持原序（保住“后绘制先触发”） */
static void sort_event_nodes(void)
{
    for (int i = 1; i < arrlen(s_event_nodes); i++) {
        Node *node = s_event_nodes[i];
        int j = i - 1;
        while (j >= 0 && s_event_nodes[j]->z_index < node->z_index) {
            s_event_nodes[j + 1] = s_event_nodes[j];
            j--;
        }
        s_event_nodes[j + 1] = node;
    }
}

/* 求 node 在以 root 为根的树中的世界坐标（沿祖先链累加 rect.x/y 偏移） */
static bool find_world_rect(Node *n, Node *target, float ox, float oy, SDL_FRect *out)
{
    if (!n)
        return false;

    float x = n->rect.x + ox;
    float y = n->rect.y + oy;
    if (n == target) {
        out->x = x;
        out->y = y;
        out->w = n->rect.w;
        out->h = n->rect.h;
        return true;
    }
    for (Node *c = n->children; c; c = c->next)
        if (find_world_rect(c, target, x, y, out))
            return true;
    return false;
}

static SDL_FRect node_world_rect(Node *root, Node *node)
{
    SDL_FRect w = { 0.0f, 0.0f, 0.0f, 0.0f };
    find_world_rect(root, node, 0.0f, 0.0f, &w);
    return w;
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
    for (int i = 0; i < arrlen(s_event_nodes); i++) {
        SDL_FRect wr = node_world_rect(root, s_event_nodes[i]);
        if (hit_node(renderer, event, &wr))
            return s_event_nodes[i];
    }
    return NULL;
}

/* 把世界坐标临时写入 node->rect，调用现有单节点分发再还原（复用 enter/leave/click 等逻辑） */
static void fire_node(SDL_Renderer *renderer, SDL_Event *event, Node *root, Node *node)
{
    SDL_FRect wr = node_world_rect(root, node);
    SDL_FRect saved = node->rect;
    node->rect = wr;
    mouseevent(renderer, event, node);
    node->rect = saved;
}

/* 构建/重建 root 的事件表：清空 → 后序收集 → z 稳定排序。
   树初始化后调用一次即可；root 变化或 Node_AddChild / Node_SetVisible 后会自动重建 */
void mouseevent_init(Node *root)
{
    if (!root)
        return;

    if (root != s_last_root)
        s_hover_focus = NULL; /* 换了场景根，丢弃旧悬停焦点，避免指向已释放节点 */
    s_last_root = root;

    arrfree(s_event_nodes);
    s_event_nodes = NULL;
    collect_event_nodes(root);
    sort_event_nodes();
}

/* 统一分发入口：场景 event() 每帧对其 root 调用一次 */
void mouseevent_dispatch(SDL_Renderer *renderer, SDL_Event *event, Node *root)
{
    if (!renderer || !event || !root)
        return;

    /* 只处理鼠标类事件：其余（键盘/退出等）一律忽略；场景只需把每个事件都转交给这里 */
    switch (event->type) {
    case SDL_EVENT_MOUSE_MOTION:
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP:
    case SDL_EVENT_MOUSE_WHEEL:
        break;
    default:
        return;
    }

    /* 事件表在 mouseevent_build（初始化）构建；这里只在 root 变化或树脏（增删子/可见性变更）时重建，
       避免每次鼠标事件都遍历整棵树 */
    if (root != s_last_root || s_dirty) {
        s_dirty = false;
        mouseevent_init(root);
    }

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
    for (int i = 0; i < arrlen(s_event_nodes); i++) {
        Node *n = s_event_nodes[i];
        SDL_FRect wr = node_world_rect(root, n);
        if (!hit_node(renderer, event, &wr))
            continue;
        fire_node(renderer, event, root, n);
        if (n->event.stop_propagation)
            break;
    }
}
