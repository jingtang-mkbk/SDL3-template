#include "minesweeper_scene.h"
#include "manager/manager.h"

static void back_clicked(void *userdata)
{
  AppState *state = (AppState *)userdata;
  state->switch_scene(state, "main_scene");
}

static void init(AppState *state)
{
  MinesweeperSceneData *d = (MinesweeperSceneData *)SDL_calloc(1, sizeof(MinesweeperSceneData));
  state->scene_data = d;

  d->hover_row = -1;
  d->hover_col = -1;

  /* 初始化 Back 文字 */
  d->back_tex = (Text){
      .base = (Element){
          .rect = {5, 5, 0, 0},
          .on_click = back_clicked,
          .event_userdata = state,
      },
      .text = "Back",
      .path = "MSYH.TTC",
      .font_size = 24.0f,
      .color = (SDL_Color){255, 255, 255, 255},
  };
  text_init(manager, &d->back_tex);

  SDL_IOStream *io = SDL_IOFromFile("assets/minesweeper_scene.json", "r");
  if (!io)
    return;

  Sint64 size = SDL_GetIOSize(io);
  char *buf = (char *)SDL_malloc(size + 1);
  SDL_ReadIO(io, buf, size);
  buf[size] = '\0';
  SDL_CloseIO(io);

  cJSON *root = cJSON_Parse(buf);
  SDL_free(buf);
  if (!root)
    return;

  cJSON *comps = cJSON_GetObjectItem(root, "components");
  cJSON *comp;
  cJSON_ArrayForEach(comp, comps)
  {
    cJSON *type = cJSON_GetObjectItem(comp, "type");
    if (!cJSON_IsString(type))
      continue;

    if (SDL_strcmp(type->valuestring, "TEXT") == 0)
    {
      cJSON *bt = cJSON_GetObjectItem(comp, "text");
      if (cJSON_IsString(bt))
        d->back_tex.text = bt->valuestring;
      cJSON *bc = cJSON_GetObjectItem(comp, "color");
      if (cJSON_IsString(bc))
      {
        Color c = parse_color(bc->valuestring);
        d->back_tex.color = (SDL_Color){c.r, c.g, c.b, 255};
      }
      cJSON *bx = cJSON_GetObjectItem(comp, "x");
      if (cJSON_IsNumber(bx))
        d->back_tex.base.rect.x = (float)bx->valuedouble;
      cJSON *by = cJSON_GetObjectItem(comp, "y");
      if (cJSON_IsNumber(by))
        d->back_tex.base.rect.y = (float)by->valuedouble;
    }
    else if (SDL_strcmp(type->valuestring, "RECT") == 0)
    {
      cJSON *cx = cJSON_GetObjectItem(comp, "x");
      cJSON *cy = cJSON_GetObjectItem(comp, "y");
      if (cJSON_IsNumber(cx))
        d->parent_x = (float)cx->valuedouble;
      if (cJSON_IsNumber(cy))
        d->parent_y = (float)cy->valuedouble;

      d->children = cJSON_GetObjectItem(comp, "children");
      if (d->children)
      {
        cJSON *fst = cJSON_GetArrayItem(d->children, 0);
        int fy = cJSON_GetObjectItem(fst, "y")->valueint;
        d->grid_cols = 0;
        cJSON *ch;
        cJSON_ArrayForEach(ch, d->children)
        {
          if (cJSON_GetObjectItem(ch, "y")->valueint == fy)
            d->grid_cols++;
          else
            break;
        }
      }
    }
  }
}

static void event(AppState *state, SDL_Event *event)
{
  MinesweeperSceneData *d = SCENE_DATA(state, MinesweeperSceneData);

  if (event->type == SDL_EVENT_KEY_DOWN ||
      event->type == SDL_EVENT_QUIT)
  {
    state->app_quit = SDL_APP_SUCCESS;
    return;
  }

  if (event->type == SDL_EVENT_MOUSE_MOTION && d->children)
  {
    float mx = event->motion.x, my = event->motion.y;

    d->hover_row = -1;
    d->hover_col = -1;
    int i = 0;
    cJSON *ch;
    cJSON_ArrayForEach(ch, d->children)
    {
      cJSON *jx = cJSON_GetObjectItem(ch, "x");
      cJSON *jy = cJSON_GetObjectItem(ch, "y");
      cJSON *jw = cJSON_GetObjectItem(ch, "w");
      cJSON *jh = cJSON_GetObjectItem(ch, "h");
      float cx = d->parent_x + (cJSON_IsNumber(jx) ? (float)jx->valuedouble : 0);
      float cy = d->parent_y + (cJSON_IsNumber(jy) ? (float)jy->valuedouble : 0);
      float cw = cJSON_IsNumber(jw) ? (float)jw->valuedouble : 51;
      float ch_val = cJSON_IsNumber(jh) ? (float)jh->valuedouble : 51;
      if (mx >= cx && mx <= cx + cw && my >= cy && my <= cy + ch_val)
      {
        d->hover_row = i / d->grid_cols;
        d->hover_col = i % d->grid_cols;
      }
      i++;
    }
  }

  if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
      event->button.button == SDL_BUTTON_LEFT && d->children)
  {
    SDL_ConvertEventToRenderCoordinates(state->renderer, event);
    float mx = event->button.x, my = event->button.y;

    /* Back */
    if (element_handle_mouseevent(&d->back_tex.base, mx, my, ELEMENT_CLICK))
      return;

    /* 方块：直接用 JSON 的 x, y, w, h */
    int i = 0;
    cJSON *ch;
    cJSON_ArrayForEach(ch, d->children)
    {
      cJSON *jx = cJSON_GetObjectItem(ch, "x");
      cJSON *jy = cJSON_GetObjectItem(ch, "y");
      cJSON *jw = cJSON_GetObjectItem(ch, "w");
      cJSON *jh = cJSON_GetObjectItem(ch, "h");
      float cx = d->parent_x + (cJSON_IsNumber(jx) ? (float)jx->valuedouble : 0);
      float cy = d->parent_y + (cJSON_IsNumber(jy) ? (float)jy->valuedouble : 0);
      float cw = cJSON_IsNumber(jw) ? (float)jw->valuedouble : 51;
      float ch_val = cJSON_IsNumber(jh) ? (float)jh->valuedouble : 51;
      if (mx >= cx && mx <= cx + cw && my >= cy && my <= cy + ch_val)
      {
        cJSON *bg = cJSON_GetObjectItem(ch, "background_color");
        Color c = cJSON_IsString(bg) ? parse_color(bg->valuestring) : (Color){0, 0, 0};
        SDL_Log("block clicked: row: %d col: %d, color: #%02X%02X%02X",
                i / d->grid_cols, i % d->grid_cols, c.r, c.g, c.b);
        return;
      }
      i++;
    }
  }
}

static void iterate(AppState *state)
{
  MinesweeperSceneData *d = SCENE_DATA(state, MinesweeperSceneData);
  // int w, h;
  // SDL_GetCurrentRenderOutputSize(state->renderer, &w, &h);

  SDL_SetRenderDrawColor(state->renderer, 20, 20, 30, 255);
  SDL_RenderClear(state->renderer);

  if (d->children)
  {
    SDL_SetRenderDrawBlendMode(state->renderer, SDL_BLENDMODE_BLEND);
    int i = 0;
    cJSON *ch;
    cJSON_ArrayForEach(ch, d->children)
    {
      int row = i / d->grid_cols, col = i % d->grid_cols;

      cJSON *bg = cJSON_GetObjectItem(ch, "background_color");
      Color c = cJSON_IsString(bg) ? parse_color(bg->valuestring) : (Color){0, 0, 0};

      cJSON *op = cJSON_GetObjectItem(ch, "opacity");
      float opv = cJSON_IsNumber(op) ? (float)op->valuedouble : 1.0f;
      Uint8 a = (row == d->hover_row && col == d->hover_col) ? (Uint8)(opv * 255) : 255;

      cJSON *jx = cJSON_GetObjectItem(ch, "x");
      cJSON *jy = cJSON_GetObjectItem(ch, "y");
      cJSON *jw = cJSON_GetObjectItem(ch, "w");
      cJSON *jh = cJSON_GetObjectItem(ch, "h");
      SDL_FRect rect = {
          d->parent_x + (cJSON_IsNumber(jx) ? (float)jx->valuedouble : 0),
          d->parent_y + (cJSON_IsNumber(jy) ? (float)jy->valuedouble : 0),
          cJSON_IsNumber(jw) ? (float)jw->valuedouble : 51,
          cJSON_IsNumber(jh) ? (float)jh->valuedouble : 51,
      };

      SDL_SetRenderDrawColor(state->renderer, c.r, c.g, c.b, a);
      SDL_RenderFillRect(state->renderer, &rect);
      i++;
    }
    SDL_SetRenderDrawBlendMode(state->renderer, SDL_BLENDMODE_NONE);
  }

  /* 文字渲染 */
  text_render(state->renderer, manager, &d->back_tex);

  SDL_RenderPresent(state->renderer);
}

static void deinit(AppState *state)
{
  SDL_free(state->scene_data);
  text_deinit();
  state->scene_data = NULL;
}

const Scene minesweeper_scene = {
    .init = init,
    .event = event,
    .iterate = iterate,
    .deinit = deinit,
};
