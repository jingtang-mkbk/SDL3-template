#include "main_scene.h"

static void switch_clicked(void *userdata)
{
  AppState *state = (AppState *)userdata;
  state->switch_scene(state, "minesweeper_scene");
}

static void tiger_clicked(void *userdata)
{
  SDL_Log("Tiger clicked!\n");
}

static void init(AppState *state)
{
  MainSceneData *d = (MainSceneData *)SDL_calloc(1, sizeof(MainSceneData));
  state->scene_data = d;

  /* 注册回调（id 只写一次） */
  shput(click_map, "1", ((ClickCB){switch_clicked, state}));
  shput(click_map, "2", ((ClickCB){tiger_clicked, state}));

  /* 从 JSON 解析元素 */
  d->arr = render_json2element("assets/json/main_scene.json");

  render_init(state->renderer, d->arr);

  /* 用 JSON 中的 id 自动绑定回调（不重复写 id） */
  for (int i = 0; i < arrlen(d->arr); i++)
  {
    if (!d->arr[i].id)
      continue;
    ClickCB cb = shget(click_map, d->arr[i].id);
    if (cb.fn)
      render_set_callback(d->arr, d->arr[i].id, cb.fn, cb.userdata);
  }

  /* Switch 居中 */
  Render_Element *sw = render_find_by_id(d->arr, "1");
  if (sw)
  {
    Element *base = (Element *)sw->element;
    int pw, ph;
    SDL_GetCurrentRenderOutputSize(state->renderer, &pw, &ph);
    base->rect.x = (pw - base->rect.w) / 2.0f;
    base->rect.y = (ph - base->rect.h) / 2.0f;
  }

  /* Start background music */
  manager.get_managers()->music_manager->load("the_entertainer.ogg");
  manager.get_managers()->music_manager->play("the_entertainer.ogg");
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
    SDL_ConvertEventToRenderCoordinates(state->renderer, event);
    float mx = event->button.x;
    float my = event->button.y;

    for (int i = 0; i < arrlen(d->arr); i++)
      element_handle_mouseevent(d->arr[i].element, mx, my, ELEMENT_CLICK);
  }
}

static void iterate(AppState *state)
{
  MainSceneData *d = SCENE_DATA(state, MainSceneData);

  SDL_SetRenderDrawColor(state->renderer, 50, 50, 50, 255);
  SDL_RenderClear(state->renderer);
  render(state->renderer, d->arr);
  SDL_RenderPresent(state->renderer);
}

static void deinit(AppState *state)
{
  MainSceneData *d = SCENE_DATA(state, MainSceneData);
  manager.get_managers()->music_manager->pause("the_entertainer.ogg");
  for (int i = 0; i < arrlen(d->arr); i++)
    SDL_free(d->arr[i].element);
  render_deinit();
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
