#include "test_scene.h"
#include "cJSON.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void init(AppState *state)
{
  TestSceneData *d = (TestSceneData *)SDL_calloc(1, sizeof(TestSceneData));
  state->scene_data = d;

  d->hover_row = -1;
  d->hover_col = -1;

  SDL_strlcpy(d->back.text, "Back", sizeof(d->back.text));
  d->back.color = (Color){255, 255, 255};
  d->back.x = 5;
  d->back.y = 5;
  d->text_scale = SDL_GetWindowDisplayScale(state->window) * 2.0f;

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
        SDL_strlcpy(d->back.text, bt->valuestring, 32);
      cJSON *bc = cJSON_GetObjectItem(comp, "color");
      if (cJSON_IsString(bc))
        d->back.color = parse_color(bc->valuestring);
      cJSON *bx = cJSON_GetObjectItem(comp, "x");
      if (cJSON_IsNumber(bx))
        d->back.x = (float)bx->valuedouble;
      cJSON *by = cJSON_GetObjectItem(comp, "y");
      if (cJSON_IsNumber(by))
        d->back.y = (float)by->valuedouble;
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

  d->text_w = SDL_strlen(d->back.text) * SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE;
  d->text_h = SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE;
}

static void event(AppState *state, SDL_Event *event)
{
  TestSceneData *d = SCENE_DATA(state, TestSceneData);

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
      cJSON *jw = cJSON_GetObjectItem(ch, "width");
      cJSON *jh = cJSON_GetObjectItem(ch, "height");
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

  if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN && d->children)
  {
    SDL_ConvertEventToRenderCoordinates(state->renderer, event);
    float mx = event->button.x, my = event->button.y;

    /* Back */
    float s = d->text_scale;
    float bx = d->back.x * s, by = d->back.y * s;
    float bw = d->text_w * s, bh = d->text_h * s;
    if (mx >= bx && mx <= bx + bw && my >= by && my <= by + bh)
    {
      state->switch_scene(state, "main_scene");
      return;
    }

    /* 方块：直接用 JSON 的 x, y, w, h */
    int i = 0;
    cJSON *ch;
    cJSON_ArrayForEach(ch, d->children)
    {
      cJSON *jx = cJSON_GetObjectItem(ch, "x");
      cJSON *jy = cJSON_GetObjectItem(ch, "y");
      cJSON *jw = cJSON_GetObjectItem(ch, "width");
      cJSON *jh = cJSON_GetObjectItem(ch, "height");
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
  TestSceneData *d = SCENE_DATA(state, TestSceneData);
  int w, h;
  SDL_GetCurrentRenderOutputSize(state->renderer, &w, &h);

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
      cJSON *jw = cJSON_GetObjectItem(ch, "width");
      cJSON *jh = cJSON_GetObjectItem(ch, "height");
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

  SDL_SetRenderScale(state->renderer, d->text_scale, d->text_scale);
  Color bc = d->back.color;
  SDL_SetRenderDrawColor(state->renderer, bc.r, bc.g, bc.b, 255);
  SDL_RenderDebugText(state->renderer, d->back.x, d->back.y, d->back.text);
  SDL_SetRenderScale(state->renderer, 1.0f, 1.0f);

  SDL_RenderPresent(state->renderer);
}

static void deinit(AppState *state)
{
  SDL_free(state->scene_data);
  state->scene_data = NULL;
}

const Scene test_scene = {
    .init = init,
    .event = event,
    .iterate = iterate,
    .deinit = deinit,
};
