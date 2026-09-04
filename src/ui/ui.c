#include "ui.h"
#include "registry.h" /* ui_register/ui_unregister 声明 */

static Components *context = NULL;

static UI_Image **img_comps = NULL;
static UI_Sprite **spr_comps = NULL;
static UI_Text **txt_comps = NULL;

static const Components *getComps(void)
{
    if (!context) {
        context = (Components *)SDL_calloc(1, sizeof(Components));
        if (!context) {
            SDL_Log("Failed to allocate memory for Components instance.");
            return NULL;
        }
    }
    return context;
}

static void init(void)
{
    const Components *comps = getComps();
    if (!comps)
        return;

    context->image = &image;
    context->sprite = &sprite;
    context->text = &text;
}

/* 登记：控件 create 成功后加入对应全局注册表（只被 create 调用一次） */
void ui_register(Node *node)
{
    if (!node)
        return;

    switch (node->type) {
    case NODETYPE_IMAGE:
        arrput(img_comps, (UI_Image *)node);
        break;
    case NODETYPE_SPRITE:
        arrput(spr_comps, (UI_Sprite *)node);
        break;
    case NODETYPE_TEXT:
        arrput(txt_comps, (UI_Text *)node);
        break;
    }
}

/* 按指针从对应注册表移除（不释放内存），供各控件 deinit 提前释放时避免二次释放 */
static void img_comps_remove(UI_Image *img)
{
    for (int i = 0; i < arrlen(img_comps); i++) {
        if (img_comps[i] == img) {
            arrdel(img_comps, i);
            return;
        }
    }
}

static void spr_comps_remove(UI_Sprite *spr)
{
    for (int i = 0; i < arrlen(spr_comps); i++) {
        if (spr_comps[i] == spr) {
            arrdel(spr_comps, i);
            return;
        }
    }
}

static void txt_comps_remove(UI_Text *txt)
{
    for (int i = 0; i < arrlen(txt_comps); i++) {
        if (txt_comps[i] == txt) {
            arrdel(txt_comps, i);
            return;
        }
    }
}

void ui_unregister(Node *node)
{
    if (!node)
        return;

    switch (node->type) {
    case NODETYPE_IMAGE:
        img_comps_remove((UI_Image *)node);
        break;
    case NODETYPE_SPRITE:
        spr_comps_remove((UI_Sprite *)node);
        break;
    case NODETYPE_TEXT:
        txt_comps_remove((UI_Text *)node);
        break;
    }
}

/* 递归渲染：node->rect.x/y 视为相对父节点的偏移，沿祖先链累加得到屏幕坐标 */
static void render_node(SDL_Renderer *renderer, Node *node, float ox, float oy)
{
    if (!node || !node->visible)
        return;

    float x = node->rect.x + ox;
    float y = node->rect.y + oy;

    /* 容器节点不绘制自身，仅作分组；其余按 type 分派到对应组件 render */
    if (node->type != NODETYPE_CONTAINER) {
        /* 组件 render 内部读取 node->rect 绘制 → 临时写入绝对坐标，画完还原为局部偏移 */
        SDL_FRect saved = node->rect;
        node->rect.x = x;
        node->rect.y = y;

        switch (node->type) {
        case NODETYPE_IMAGE:
            context->image->render(renderer, (UI_Image *)node);
            break;
        case NODETYPE_SPRITE:
            context->sprite->render(renderer, (UI_Sprite *)node);
            break;
        case NODETYPE_TEXT:
            context->text->render(renderer, (UI_Text *)node);
            break;
        default:
            break;
        }

        node->rect = saved;
    }

    /* 递归子节点：以本节点绝对位置为基准继续累加偏移 */
    for (Node *child = node->children; child; child = child->next)
        render_node(renderer, child, x, y);
}

/* 渲染一棵以 root 为根的控件树（root 的 rect 为屏幕坐标） */
static void render(SDL_Renderer *renderer, Node *root)
{
    if (!renderer || !root)
        return;

    render_node(renderer, root, 0.0f, 0.0f);
}

/* 递归释放一个节点：先释放其子树，再按类型经组件 deinit 注销并释放；
   image/sprite 贴图归 texture_manager 缓存，离开场景时一并从缓存移除（切回自动重载） */
static void release_node(Node *n)
{
    if (!n)
        return;

    /* 先释放子节点（先保存 next，避免节点释放后访问悬垂指针） */
    Node *child = n->children;
    while (child) {
        Node *next = child->next;
        release_node(child);
        child = next;
    }

    const Managers *mgr = manager.get_managers();
    switch (n->type) {
    case NODETYPE_IMAGE:
        UI_Image *img = (UI_Image *)n;
        if (mgr && mgr->texture_manager)
            mgr->texture_manager->remove(img->path);
        if (context->image)
            context->image->deinit(img);
        break;
    case NODETYPE_SPRITE:
    {
        UI_Sprite *sprite = (UI_Sprite *)n;
        if (mgr && mgr->texture_manager)
            mgr->texture_manager->remove_sprite(sprite->path, sprite->count);
        if (context->sprite)
            context->sprite->deinit(sprite);
        break;
    }
    case NODETYPE_TEXT:
        UI_Text *txt = (UI_Text *)n;
        if (context->text)
            context->text->deinit(txt);
        break;
    default: /* NODETYPE_CONTAINER 或裸节点：只释放结构体 */
        if (n->event.userevent_arr)
            arrfree(n->event.userevent_arr);
        SDL_free(n);
        break;
    }
}

/* 释放 root 及其下的所有组件（场景在 deinit 时调用一次即可） */
static void release_root(Node *root)
{
    if (!root)
        return;

    release_node(root);
}

/* 应用退出时统一释放所有登记控件（由 SDL_AppQuit 调用） */
static void deinit(void)
{
    /* Image/Sprite 贴图归 texture_manager 缓存所有，此处只释放结构体与事件数组 */
    for (int i = 0; i < arrlen(img_comps); i++) {
        arrfree(img_comps[i]->node.event.userevent_arr);
        SDL_free(img_comps[i]);
    }
    arrfree(img_comps);
    img_comps = NULL;

    for (int i = 0; i < arrlen(spr_comps); i++) {
        arrfree(spr_comps[i]->node.event.userevent_arr);
        SDL_free(spr_comps[i]);
    }
    arrfree(spr_comps);
    spr_comps = NULL;

    /* Text 贴图为控件自建，需先销毁再释放结构体 */
    for (int i = 0; i < arrlen(txt_comps); i++) {
        SDL_DestroyTexture(txt_comps[i]->texture);
        arrfree(txt_comps[i]->node.event.userevent_arr);
        SDL_free(txt_comps[i]);
    }
    arrfree(txt_comps);
    txt_comps = NULL;

    SDL_free(context);
    context = NULL;
}

const UI ui = {
    .init = init,
    .deinit = deinit,
    .render = render,
    .release_root = release_root,
    .getComps = getComps,
};