#include "gomoku_scene.h"

static int preWidth, preHeight;

static void init(AppState *state)
{
    GomokuSceneData *d = (GomokuSceneData *)SDL_calloc(1, sizeof(GomokuSceneData));
    state->scene_data = d;
    SDL_GetWindowSize(state->window, &preWidth, &preHeight);
    SDL_SetWindowSize(state->window, 800, 800);
    SDL_SetRenderDrawBlendMode(state->renderer, SDL_BLENDMODE_BLEND);

    // 背景
    d->background_img =
        UI_Image_CreateWithProp(state->renderer, "gomoku.png", (SDL_FRect){ 0, 0, 800, 800 });

    // 白子
    d->whiteArr =
        UI_Image_CreateWithProp(state->renderer, "white.png", (SDL_FRect){ 100, 100, 40, 40 });

    // 黑子
}

static void event(AppState *state, SDL_Event *event)
{
    GomokuSceneData *d = SCENE_DATA(state, GomokuSceneData);

    if (event->type == SDL_EVENT_QUIT) {
        state->app_quit = SDL_APP_SUCCESS;
        return;
    }
}

static void iterate(AppState *state)
{
    GomokuSceneData *d = SCENE_DATA(state, GomokuSceneData);

    SDL_SetRenderDrawColor(state->renderer, 50, 50, 50, 255);
    SDL_RenderClear(state->renderer);
    UI_Image_Render(state->renderer, d->background_img);
    UI_Image_Render(state->renderer, d->whiteArr);

    SDL_RenderPresent(state->renderer);
}

static void deinit(AppState *state)
{
    UI_Image_Deinit();
    SDL_free(state->scene_data);
    state->scene_data = NULL;
    SDL_SetWindowSize(state->window, preWidth, preHeight);
}

const Scene gomoku_scene = {
    .init = init,
    .event = event,
    .iterate = iterate,
    .deinit = deinit,
};