#include "main_scene.h"
#include "ui/ui.h"

static void minesweeper_clicked(SDL_Event *event, void *userdata)
{
    AppState *state = (AppState *)userdata;
    state->switch_scene(state, "minesweeper_scene");
}

static void mousedown(SDL_Event *event, void *userdata)
{
    SDL_Log("Test mousedown\n");
}

static void mouseup(SDL_Event *event, void *userdata)
{
    SDL_Log("Test mouseup\n");
}

static void mouseenter(SDL_Event *event, void *userdata)
{
    SDL_Log("Test mouseenter\n");
}

static void mouseleave(SDL_Event *event, void *userdata)
{
    SDL_Log("Test mouseleave\n");
}

static void mousemove(SDL_Event *event, void *userdata)
{
    SDL_Log("Test mousemove\n");
}

static void mousehover(SDL_Event *event, void *userdata)
{
    SDL_Log("Test mousehover\n");
}

static void click(SDL_Event *event, void *userdata)
{
    SDL_Log("Test click\n");
}

static void click_right(SDL_Event *event, void *userdata)
{
    SDL_Log("Test click_right\n");
}

static void mousedown_right(SDL_Event *event, void *userdata)
{
    SDL_Log("Test mousedown_right\n");
}

static void mouseup_right(SDL_Event *event, void *userdata)
{
    SDL_Log("Test mouseup_right\n");
}

static void tiger_clicked(SDL_Event *event, void *userdata)
{
    SDL_Log("Tiger clicked!\n");
}

static void gomoku_clicked(SDL_Event *event, void *userdata)
{
    AppState *state = (AppState *)userdata;
    state->switch_scene(state, "gomoku_scene");
}

static void init(AppState *state)
{
    MainSceneData *d = (MainSceneData *)SDL_calloc(1, sizeof(MainSceneData));
    state->scene_data = d;

    d->MineSweeper = ui.getComps()->text->createWithClick((SDL_FRect){ 0, 0, 0, 0 }, "Mine Sweeper", NULL, 48, TextAlign_None, state, minesweeper_clicked);
    /* 复合字面量直接作为参数传（mouseEvent.setMultiMouseEvent 内部会拷贝，临时对象仅需存活到调用结束） */
    d->Test = ui.getComps()->text->createWithMultiEvent(
        (SDL_FRect){ 0, 60, 0, 0 }, "Test", NULL, 48, TextAlign_None, state,
        (Event_Userevent[]){
            { MOUSEEVENT_DOWN, mousedown },
            { MOUSEEVENT_UP, mouseup },
            { MOUSEEVENT_ENTER, mouseenter },
            { MOUSEEVENT_LEAVE, mouseleave },
            { MOUSEEVENT_CLICK, click },
            { MOUSEEVENT_CLICK_RIGHT, click_right },
            { MOUSEEVENT_DOWN_RIGHT, mousedown_right },
            { MOUSEEVENT_UP_RIGHT, mouseup_right },
        },
        8);
    d->Gomoku = ui.getComps()->text->create((SDL_FRect){ 0, 120, 0, 0 }, "Gomoku", NULL, 48, TextAlign_None);
    d->Tiger = ui.getComps()->image->create(state->renderer, "imgs/gs_tiger.svg", (SDL_FRect){ 100, 100, 200, 200 });
    d->SlimeGreen = ui.getComps()->sprite->create(state->renderer, "sprites/SlimeGreen/SlimeBasic_00", (SDL_FRect){ 0, 100, 376, 256 }, 30, 2000);
    d->SlimeOrange = ui.getComps()->sprite->create(state->renderer, "sprites/SlimeOrange/SlimeOrange_00", (SDL_FRect){ 0, 300, 510, 410 }, 30, 2000);
    // ui.getComps()->sprite->setAlpha(d->SlimeGreen, 0.5f);
    ui.getComps()->sprite->setAlpha(d->SlimeOrange, 0.5f);
    ui.getComps()->text->setAlpha(d->MineSweeper, 0.5f);
    ui.getComps()->sprite->setTimeScale(d->SlimeGreen, 5.0f);

    /* 居中 */
    int pw, ph;
    SDL_GetCurrentRenderOutputSize(state->renderer, &pw, &ph);
    d->MineSweeper->node.rect.x = (pw - d->MineSweeper->node.rect.w) / 2.0f;
    d->Test->node.rect.x = (pw - d->Test->node.rect.w) / 2.0f;
    d->Gomoku->node.rect.x = (pw - d->Gomoku->node.rect.w) / 2.0f;

    // mouseEvent.setClickWithUserdata((Node *)d->Test, state, test_mousedown);
    events.mouse->setClickWithUserdata((Node *)d->Gomoku, state, gomoku_clicked);

    /* 建立控件树：root 为容器(0,0)。子节点 rect 现为屏幕坐标，作为相对 root 的偏移，视觉不变；
       移动 root 即可整体平移子节点 */
    d->root = Node_Create();
    Node_Default(d->root, NODETYPE_CONTAINER);
    d->root->rect = (SDL_FRect){ 0.0f, 0.0f, (float)pw, (float)ph };
    /* 追加顺序即绘制顺序，后追加的叠在上面（与旧的手动渲染顺序一致） */
    Node_AddChild(d->root, (Node *)d->Tiger);
    Node_AddChild(d->root, (Node *)d->MineSweeper);
    Node_AddChild(d->root, (Node *)d->Test);
    Node_AddChild(d->root, (Node *)d->Gomoku);
    Node_AddChild(d->root, (Node *)d->SlimeGreen);
    Node_AddChild(d->root, (Node *)d->SlimeOrange);

    /* 一次性构建事件表（收集+排序放在初始化，不在每次 event 里做） */
    events.mouse->build(d->root);

    /* Start background music（play 未加载时会自动 load） */
    manager.get_managers()->music_manager->play("the_entertainer.ogg");
}

static void event(AppState *state, SDL_Event *event)
{
    MainSceneData *d = SCENE_DATA(state, MainSceneData);

    if (event->type == SDL_EVENT_QUIT) {
        state->app_quit = SDL_APP_SUCCESS;
        return;
    }

    /* 事件处理封装在 mouse_event（内部过滤鼠标事件类型）；渲染走 ui，事件走 mouseEvent */
    mouseEvent.dispatch(state->renderer, event, d->root);
}

static void iterate(AppState *state)
{
    MainSceneData *d = SCENE_DATA(state, MainSceneData);

    SDL_SetRenderDrawColor(state->renderer, 50, 50, 50, 255);
    SDL_RenderClear(state->renderer);
    /* 递归渲染整棵控件树（子节点 rect 为相对父偏移，由 ui.render 累加） */
    ui.render(state->renderer, d->root);
    SDL_RenderPresent(state->renderer);
}

static void deinit(AppState *state)
{
    MainSceneData *d = SCENE_DATA(state, MainSceneData);
    /* 统一释放整棵控件树：注销注册表、销毁 text 贴图、移除 image/sprite 贴图缓存（切回自动重载） */
    ui.release_root(d->root);
    SDL_free(state->scene_data);
    state->scene_data = NULL;
}

const Scene main_scene = {
    .init = init,
    .event = event,
    .iterate = iterate,
    .deinit = deinit,
};
