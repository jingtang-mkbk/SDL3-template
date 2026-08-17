#include "main_scene.h"

static void switch_clicked(SDL_Event *event, void *userdata)
{
    AppState *state = (AppState *)userdata;
    state->switch_scene(state, "minesweeper_scene");
}

static void test_clicked(SDL_Event *event, void *userdata)
{
    SDL_Log("Test clicked\n");
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

    /* 注册回调（id 只写一次） */
    shput(click_map, "1", ((ClickCB){ switch_clicked, state }));
    shput(click_map, "2", ((ClickCB){ test_clicked, state }));
    shput(click_map, "3", ((ClickCB){ tiger_clicked, state }));
    shput(click_map, "Gomoku", ((ClickCB){ gomoku_clicked, state }));

    /* 从 JSON 解析元素 */
    d->arr = Render_json2element(state->renderer, "assets/json/main_scene.json");

    Render_init(state->renderer, d->arr);

    /* 用 JSON 中的 id 自动绑定回调（不重复写 id） */
    for (int i = 0; i < arrlen(d->arr); i++) {
        if (!d->arr[i].id)
            continue;
        ClickCB cb = shget(click_map, d->arr[i].id);
        if (cb.fn)
            Render_set_callback(d->arr, d->arr[i].id, cb.fn, cb.userdata);
    }

    /* 居中 */
    int pw, ph;
    SDL_GetCurrentRenderOutputSize(state->renderer, &pw, &ph);
    const char *center_ids[] = { "1", "2", "Gomoku" };
    for (int i = 0; i < 3; i++) {
        Render_Element *el = Render_find_by_id(d->arr, center_ids[i]);
        if (el) {
            Event *base = (Event *)el->element;
            base->rect.x = (pw - base->rect.w) / 2.0f;
        }
    }

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

    for (int i = 0; i < arrlen(d->arr); i++)
        mouseevent_handle(state->renderer, event, d->arr[i].element, EVENT_CLICK);
}

static void iterate(AppState *state)
{
    MainSceneData *d = SCENE_DATA(state, MainSceneData);

    SDL_SetRenderDrawColor(state->renderer, 50, 50, 50, 255);
    SDL_RenderClear(state->renderer);
    Render_render(state->renderer, d->arr);
    SDL_RenderPresent(state->renderer);
}

static void deinit(AppState *state)
{
    MainSceneData *d = SCENE_DATA(state, MainSceneData);
    Render_deinit(d->arr);
    shfree(click_map);
    click_map = NULL;
    SDL_free(state->scene_data);
    state->scene_data = NULL;
}

const Scene main_scene = {
    .init = init,
    .event = event,
    .iterate = iterate,
    .deinit = deinit,
};
