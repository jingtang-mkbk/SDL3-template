#define SDL_MAIN_USE_CALLBACKS 1 /* use the callbacks instead of main() */
#define STB_DS_IMPLEMENTATION    /* generate stb_ds function bodies in this TU */

#include <SDL3/SDL_main.h>
#include "manager/manager.h"
#include "scenes/main_scene/main_scene.h"
#include "scenes/minesweeper_scene/minesweeper_scene.h"
#include "platform/platform.h"

/* Scene switcher: call with scene name string to request a switch next frame */
void switch_scene(AppState *state, const char *name)
{
    const Scene *s = shget(state->scene_map, name);
    if (s)
        state->next_scene = s;
    else
        SDL_Log("switch_scene: unknown scene '%s'", name);
}

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    SDL_WindowFlags flag = 0;
    int width, height;

    /* Must allocate on heap, stack variable would be destroyed after return */
    AppState *state = (AppState *)SDL_calloc(1, sizeof(AppState));
    if (!state)
    {
        return SDL_APP_FAILURE;
    }

    /* 获取平台窗口大小 */
    getWithHeightFromPlatform(&width, &height);

    /* Create the window */
    if (!SDL_CreateWindowAndRenderer("SDL3-Template", width, height, flag, &state->window, &state->renderer))
    {
        SDL_Log("Couldn't create window and renderer: %s", SDL_GetError());
        SDL_free(state);
        return SDL_APP_FAILURE;
    }

    manager.init(MANAGER_FLAG_FONT | MANAGER_FLAG_IMAGE | MANAGER_FLAG_MUSIC);

    /* Build scene hash map: string name → Scene* */
    state->scene_map = NULL;
    shput(state->scene_map, "main_scene", &main_scene);
    shput(state->scene_map, "minesweeper_scene", &minesweeper_scene);

    /* Boot the first scene */
    state->current_scene = &main_scene;
    state->next_scene = &main_scene;
    state->current_scene->init(state);
    state->switch_scene = switch_scene;

    *appstate = state;

    SDL_SetRenderVSync(state->renderer, 1); // 强制垂直同步，锁定刷新率，降低 CPU/GPU 占用
    return SDL_APP_CONTINUE;
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    AppState *state = (AppState *)appstate;
    state->current_scene->event(state, event);
    return SDL_APP_CONTINUE;
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate)
{
    AppState *state = (AppState *)appstate;

    /* Check for pending scene switch */
    if (state->next_scene != state->current_scene)
    {
        state->current_scene->deinit(state);
        state->current_scene = state->next_scene;
        state->current_scene->init(state);
    }

    state->current_scene->iterate(state);
    return state->app_quit;
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    AppState *state = (AppState *)appstate;
    if (state)
    {
        shfree(state->scene_map);
        SDL_free(state);
    }
    manager.deinit();
}
