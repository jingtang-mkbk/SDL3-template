#include "main_scene.h"

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

    d->MineSweeper = UI_Text_CreateWithClick((SDL_FRect){ 0, 0, 0, 0 }, "Mine Sweeper", NULL, 48, TextAlign_None, state, minesweeper_clicked);
    /* 复合字面量直接作为参数传（UI_SetMultiEvent 内部会拷贝，临时对象仅需存活到调用结束） */
    d->Test = UI_Text_CreateWithMultiEvent(
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
    d->Gomoku = UI_Text_Create((SDL_FRect){ 0, 120, 0, 0 }, "Gomoku", NULL, 48, TextAlign_None);
    d->Tiger = UI_Image_Create(state->renderer, "imgs/gs_tiger.svg", (SDL_FRect){ 100, 100, 200, 200 });
    d->SlimeGreen = UI_Sprite_Create(state->renderer, "sprites/SlimeGreen/SlimeBasic_00", (SDL_FRect){ 0, 100, 376, 256 }, 30, 1000);
    d->SlimeOrange = UI_Sprite_Create(state->renderer, "sprites/SlimeOrange/SlimeOrange_00", (SDL_FRect){ 0, 300, 510, 410 }, 30, 2000);

    /* 居中 */
    int pw, ph;
    SDL_GetCurrentRenderOutputSize(state->renderer, &pw, &ph);
    d->MineSweeper->base.rect.x = (pw - d->MineSweeper->base.rect.w) / 2.0f;
    d->Test->base.rect.x = (pw - d->Test->base.rect.w) / 2.0f;
    d->Gomoku->base.rect.x = (pw - d->Gomoku->base.rect.w) / 2.0f;

    // UI_SetClickWithUserdata((UI_Event *)d->Test, state, test_mousedown);
    UI_SetClickWithUserdata((UI_Event *)d->Gomoku, state, gomoku_clicked);

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

    mouseevent(state->renderer, event, (UI_Event *)d->MineSweeper);
    mouseevent(state->renderer, event, (UI_Event *)d->Test);
    mouseevent(state->renderer, event, (UI_Event *)d->Gomoku);
}

static void iterate(AppState *state)
{
    MainSceneData *d = SCENE_DATA(state, MainSceneData);

    SDL_SetRenderDrawColor(state->renderer, 50, 50, 50, 255);
    SDL_RenderClear(state->renderer);
    UI_Image_Render(state->renderer, d->Tiger);
    UI_Text_Render(state->renderer, d->MineSweeper);
    UI_Text_Render(state->renderer, d->Test);
    UI_Text_Render(state->renderer, d->Gomoku);
    UI_Sprite_Render(state->renderer, d->SlimeGreen);
    UI_Sprite_Render(state->renderer, d->SlimeOrange);
    // Render_render(state->renderer, d->arr);
    SDL_RenderPresent(state->renderer);
}

static void deinit(AppState *state)
{
    MainSceneData *d = SCENE_DATA(state, MainSceneData);
    UI_Image_Deinit();
    UI_Text_Deinit();
    UI_Sprite_Deinit();
    SDL_free(state->scene_data);
    state->scene_data = NULL;
}

const Scene main_scene = {
    .init = init,
    .event = event,
    .iterate = iterate,
    .deinit = deinit,
};
