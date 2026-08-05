#ifndef RENDER_H
#define RENDER_H

#include "cJSON.h"
#include "data.h"
#include "ui/text.h"
#include "ui/image.h"
#include "manager/manager.h"

typedef enum Render_Type
{
  Render_Type_Text,
  Render_Type_Image,
  Render_Type_Button,
} Render_Type;

typedef struct Render_Element Render_Element;

typedef struct Render_Element
{
  Uint8 type;
  char *id;
  void *element;
  Render_Element *children;
  Render_Element *parent;
} Render_Element;

typedef struct CallbackBinding
{
  const char *id;
  void (*on_click)(SDL_Event *, void *);
  void *userdata;
} CallbackBinding;

void Render_init(SDL_Renderer *renderer, Render_Element *arr);
void Render_render(SDL_Renderer *renderer, Render_Element *arr);
void Render_deinit(Render_Element *arr);
Render_Element *Render_json2element(const char *filepath);
Render_Element *Render_find_by_id(Render_Element *arr, const char *id);
void Render_set_callback(Render_Element *arr, const char *id, void (*on_click)(SDL_Event *, void *), void *userdata);
void Render_set_callbacks(Render_Element *arr, const CallbackBinding *bindings, int count);

#endif