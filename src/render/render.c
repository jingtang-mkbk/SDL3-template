#include "render.h"
#include "utils/utils.h"

Render_Element *render_find_by_id(Render_Element *arr, const char *id)
{
  for (int i = 0; i < arrlen(arr); i++)
  {
    if (arr[i].id && SDL_strcmp(arr[i].id, id) == 0)
      return &arr[i];
    if (arr[i].children)
    {
      Render_Element *found = render_find_by_id(arr[i].children, id);
      if (found)
        return found;
    }
  }
  return NULL;
}

void render_set_callback(Render_Element *arr, const char *id, void (*on_click)(SDL_Event *, void *), void *userdata)
{
  Render_Element *el = render_find_by_id(arr, id);
  if (!el || !el->element)
    return;
  Event *base = (Event *)el->element;
  base->on_click = on_click;
  base->event_userdata = userdata;
}

void render_set_callbacks(Render_Element *arr, const CallbackBinding *bindings, int count)
{
  for (int i = 0; i < count; i++)
    render_set_callback(arr, bindings[i].id, bindings[i].on_click, bindings[i].userdata);
}

Render_Element *render_json2element(const char *filepath)
{
  Render_Element *arr = NULL;
  SDL_IOStream *io = SDL_IOFromFile(filepath, "r");
  if (!io)
    return NULL;

  Sint64 size = SDL_GetIOSize(io);
  char *buf = SDL_malloc(size + 1);
  SDL_ReadIO(io, buf, size);
  buf[size] = '\0';
  SDL_CloseIO(io);

  cJSON *root = cJSON_Parse(buf);
  if (!root)
  {
    SDL_free(buf);
    return NULL;
  }

  cJSON *components = cJSON_GetObjectItem(root, "components");
  if (!cJSON_IsArray(components))
  {
    cJSON_Delete(root);
    SDL_free(buf);
    return NULL;
  }

  cJSON *comp;
  cJSON_ArrayForEach(comp, components)
  {
    cJSON *type_json = cJSON_GetObjectItem(comp, "type");
    if (!cJSON_IsString(type_json))
      continue;

    Render_Element re = {0};
    cJSON *id_json = cJSON_GetObjectItem(comp, "id");
    if (cJSON_IsString(id_json))
      re.id = SDL_strdup(id_json->valuestring);

    if (SDL_strcmp(type_json->valuestring, "TEXT") == 0)
    {
      re.type = Render_Type_Text;
      Text *text = SDL_calloc(1, sizeof(Text));
      cJSON *t = cJSON_GetObjectItem(comp, "text");
      if (cJSON_IsString(t))
        text->text = SDL_strdup(t->valuestring);
      cJSON *fs = cJSON_GetObjectItem(comp, "font_size");
      if (cJSON_IsNumber(fs))
        text->font_size = (float)fs->valuedouble;
      text->path = "MSYH.TTC";
      cJSON *color = cJSON_GetObjectItem(comp, "color");
      if (cJSON_IsString(color))
      {
        SDL_Color c = parse_color(color->valuestring);
        text->color = (SDL_Color){c.r, c.g, c.b, c.a};
      }
      re.element = text;
    }
    else if (SDL_strcmp(type_json->valuestring, "IMAGE") == 0)
    {
      re.type = Render_Type_Image;
      Image *img = SDL_calloc(1, sizeof(Image));
      cJSON *p = cJSON_GetObjectItem(comp, "path");
      if (cJSON_IsString(p))
        img->path = SDL_strdup(p->valuestring);
      re.element = img;
    }

    if (re.element)
    {
      Event *base = (Event *)re.element;
      /* 先尝试 base.rect 嵌套格式 */
      cJSON *base_json = cJSON_GetObjectItem(comp, "base");
      cJSON *rect_json = base_json ? cJSON_GetObjectItem(base_json, "rect") : NULL;
      cJSON *x = rect_json ? cJSON_GetObjectItem(rect_json, "x") : cJSON_GetObjectItem(comp, "x");
      cJSON *y = rect_json ? cJSON_GetObjectItem(rect_json, "y") : cJSON_GetObjectItem(comp, "y");
      cJSON *w = rect_json ? cJSON_GetObjectItem(rect_json, "w") : cJSON_GetObjectItem(comp, "w");
      cJSON *h = rect_json ? cJSON_GetObjectItem(rect_json, "h") : cJSON_GetObjectItem(comp, "h");
      base->rect.x = cJSON_IsNumber(x) ? (float)x->valuedouble : 0;
      base->rect.y = cJSON_IsNumber(y) ? (float)y->valuedouble : 0;
      base->rect.w = cJSON_IsNumber(w) ? (float)w->valuedouble : 0;
      base->rect.h = cJSON_IsNumber(h) ? (float)h->valuedouble : 0;
    }

    arrput(arr, re);
  }

  cJSON_Delete(root);
  SDL_free(buf);
  return arr;
}

void render_init(SDL_Renderer *renderer, Render_Element *arr)
{
  for (int i = 0; i < arrlen(arr); i++)
  {
    switch (arr[i].type)
    {
    case Render_Type_Text:
      text_init((Text *)arr[i].element);
      break;
    case Render_Type_Image:
      image_init(renderer, manager, (Image *)arr[i].element);
      break;
    }

    if (arr[i].children)
      render_init(renderer, arr[i].children);
  }
}

void render(SDL_Renderer *renderer, Render_Element *arr)
{
  for (int i = 0; i < arrlen(arr); i++)
  {
    switch (arr[i].type)
    {
    case Render_Type_Text:
      Text *text = (Text *)arr[i].element;
      text_render(renderer, text);
      if (arr[i].children)
        render(renderer, arr[i].children);
      break;
    case Render_Type_Image:
      Image *img = (Image *)arr[i].element;
      image_render(renderer, img);
      if (arr[i].children)
        render(renderer, arr[i].children);
      break;
    case Render_Type_Button:
      break;
    default:
      break;
    }
  }
}

void render_deinit(Render_Element *arr)
{
  if (!arr)
    return;

  for (int i = 0; i < arrlen(arr); i++)
  {
    SDL_free(arr[i].id); /* SDL_strdup 分配的 id */
    if (arr[i].children)
      render_deinit(arr[i].children);

    switch (arr[i].type)
    {
    case Render_Type_Text:
    {
      Text *text = (Text *)arr[i].element;
      SDL_free(text->text); /* SDL_strdup 分配的文本 */
      SDL_free(text);
      break;
    }
    case Render_Type_Image:
    {
      Image *img = (Image *)arr[i].element;
      SDL_free(img->path); /* SDL_strdup 分配的路径 */
      SDL_free(img);
      break;
    }
    default:
      SDL_free(arr[i].element);
      break;
    }
  }

  arrfree(arr);

  image_deinit();
  text_deinit();
}