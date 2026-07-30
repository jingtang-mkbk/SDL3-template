#include "main_scene.h"
#include "../../engine/music/music.h"

static void init(AppState *state)
{
  MainSceneData *d = (MainSceneData *)SDL_calloc(1, sizeof(MainSceneData));
  state->scene_data = d;

  const char *text = "Switch";
  d->text_scale = SDL_GetWindowDisplayScale(state->window) * 4.0f;
  d->text_w = SDL_strlen(text) * SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE;
  d->text_h = SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE;

  /* Load images into sprite map */
  init_sprite(&d->sprites, state->renderer,
              "tiger", "assets/imgs/gs_tiger.svg", 100, 100, 200.0f, 200.0f);

  /* Start background music */
  if (music_init("assets/music/the_entertainer.ogg"))
    music_play();
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

    float tw = d->text_w * d->text_scale;
    float th = d->text_h * d->text_scale;
    if (mx >= d->text_x && mx <= d->text_x + tw &&
        my >= d->text_y && my <= d->text_y + th)
    {
      state->switch_scene(state, "test_scene");
    }
  }
}

static void iterate(AppState *state)
{
  MainSceneData *d = SCENE_DATA(state, MainSceneData);
  int pixel_w, pixel_h;
  SDL_GetCurrentRenderOutputSize(state->renderer, &pixel_w, &pixel_h);

  float tw = d->text_w * d->text_scale;
  float th = d->text_h * d->text_scale;
  d->text_x = (pixel_w - tw) / 2.0f;
  d->text_y = (pixel_h - th) / 2.0f;

  SDL_SetRenderDrawColor(state->renderer, 50, 50, 50, 255);
  SDL_RenderClear(state->renderer);

  render_sprite(state->renderer, &d->sprites, "tiger");

  SDL_SetRenderScale(state->renderer, d->text_scale, d->text_scale);
  SDL_SetRenderDrawColor(state->renderer, 255, 255, 255, 255);
  SDL_RenderDebugText(state->renderer,
                      d->text_x / d->text_scale,
                      d->text_y / d->text_scale,
                      "Switch");
  SDL_SetRenderScale(state->renderer, 1.0f, 1.0f);

  SDL_RenderPresent(state->renderer);
}

static void deinit(AppState *state)
{
  MainSceneData *d = SCENE_DATA(state, MainSceneData);
  music_deinit();
  deinit_sprites(&d->sprites);
  SDL_free(state->scene_data);
  state->scene_data = NULL;
}

const Scene main_scene = {
    .init = init,
    .event = event,
    .iterate = iterate,
    .deinit = deinit,
};
