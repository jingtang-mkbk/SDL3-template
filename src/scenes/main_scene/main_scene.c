#include "main_scene.h"
#include "manager/manager.h"
#include "ui/text.h"

static void init(AppState *state)
{
  MainSceneData *d = (MainSceneData *)SDL_calloc(1, sizeof(MainSceneData));
  state->scene_data = d;

  // 初始化 Switch 文字
  d->switch_tex = (Text){
      .text = "Switch",
      .path = "assets/fonts/MSYH.TTC",
      .font_size = 48.0f,
      .color = (SDL_Color){255, 255, 255, 255},
  };
  text_init(manager, &d->switch_tex);
  int pixel_w, pixel_h;
  SDL_GetCurrentRenderOutputSize(state->renderer, &pixel_w, &pixel_h);
  d->switch_tex.rect.x = (pixel_w - d->switch_tex.rect.w) / 2.0f;
  d->switch_tex.rect.y = (pixel_h - d->switch_tex.rect.h) / 2.0f;

  // 初始化 tiger
  manager.get_managers()->image_manager->load(state->renderer, "assets/imgs/gs_tiger.svg");
  d->tiger_img = (Image){
      .path = "assets/imgs/gs_tiger.svg",
      .rect = {100, 100, 200, 200},
  };
  image_init(manager, &d->tiger_img);

  /* Start background music */
  manager.get_managers()->music_manager->load("assets/music/the_entertainer.ogg");
  manager.get_managers()->music_manager->play("assets/music/the_entertainer.ogg");
}

static void event(AppState *state, SDL_Event *event)
{
  MainSceneData *d = SCENE_DATA(state, MainSceneData);

  if (event->type == SDL_EVENT_QUIT)
  {
    state->app_quit = SDL_APP_SUCCESS;
    return;
  }

  if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN)
  {
    float mx = event->button.x;
    float my = event->button.y;

    SDL_ConvertEventToRenderCoordinates(state->renderer, event);
    mx = event->button.x;
    my = event->button.y;

    float tw = d->switch_tex.rect.w;
    float th = d->switch_tex.rect.h;
    if (mx >= d->switch_tex.rect.x && mx <= d->switch_tex.rect.x + tw &&
        my >= d->switch_tex.rect.y && my <= d->switch_tex.rect.y + th)
    {
      state->switch_scene(state, "minesweeper_scene");
    }
  }
}

static void iterate(AppState *state)
{
  MainSceneData *d = SCENE_DATA(state, MainSceneData);

  SDL_SetRenderDrawColor(state->renderer, 50, 50, 50, 255);
  SDL_RenderClear(state->renderer);

  image_render(state->renderer, &d->tiger_img);

  /* TTF 文字渲染 */
  text_render(state->renderer, manager, &d->switch_tex);

  SDL_RenderPresent(state->renderer);
}

static void deinit(AppState *state)
{
  MainSceneData *d = SCENE_DATA(state, MainSceneData);
  manager.get_managers()->music_manager->pause("assets/music/the_entertainer.ogg");
  image_deinit();
  text_deinit();
  SDL_free(state->scene_data);
  state->scene_data = NULL;
}

const Scene main_scene = {
    .init = init,
    .event = event,
    .iterate = iterate,
    .deinit = deinit,
};
