#include "event.h"
#include "utils/utils.h"

typedef struct Event_internal
{
  SDL_FRect rect;
  void (*on_click)(SDL_Event *event, void *userdata);
  void (*on_click_right)(SDL_Event *event, void *userdata);
  void (*on_mouseenter)(SDL_Event *event, void *userdata);
  void (*on_mouseleave)(SDL_Event *event, void *userdata);
  void *event_userdata;
  bool in_rect;
} Event_internal;

static void mouseevent_click(SDL_Renderer *renderer, SDL_Event *event, Event *base)
{
  if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN && event->button.button == SDL_BUTTON_LEFT)
  {
    SDL_ConvertEventToRenderCoordinates(renderer, event);
    float mx = event->button.x;
    float my = event->button.y;

    if (!base || !hit_in_rect(&base->rect, mx, my))
      return;

    base->on_click(event, base->event_userdata);
  }
}

static void mouseevent_rclick(SDL_Renderer *renderer, SDL_Event *event, Event *base)
{
  if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN && event->button.button == SDL_BUTTON_RIGHT)
  {
    SDL_ConvertEventToRenderCoordinates(renderer, event);
    float mx = event->button.x;
    float my = event->button.y;

    if (!base || !hit_in_rect(&base->rect, mx, my))
      return;

    base->on_click_right(event, base->event_userdata);
  }
}

static void mouseevent_motion(SDL_Renderer *renderer, SDL_Event *event, Event *_base, const Uint8 event_flags)
{
  if (!_base || event->type != SDL_EVENT_MOUSE_MOTION)
    return;

  Event_internal *base = (Event_internal *)_base;
  SDL_ConvertEventToRenderCoordinates(renderer, event);
  float mx = event->button.x;
  float my = event->button.y;

  bool inside = hit_in_rect(&base->rect, mx, my);

  if ((event_flags & EVENT_HOVER) && inside && base->on_mouseenter)
    base->on_mouseenter(event, base->event_userdata);
  else if ((event_flags & EVENT_LEAVE) && !inside && base->in_rect && base->on_mouseleave)
    base->on_mouseleave(event, base->event_userdata);

  base->in_rect = inside;
}

void mouseevent_handle(SDL_Renderer *renderer, SDL_Event *event, Event *base, const Uint8 event_flags)
{
  if (!base || !event)
    return;
  if ((event_flags & EVENT_CLICK) && base->on_click)
    mouseevent_click(renderer, event, base);
  if ((event_flags & EVENT_CLICK_RIGHT) && base->on_click_right)
    mouseevent_rclick(renderer, event, base);
  if ((event_flags & (EVENT_HOVER | EVENT_LEAVE)) && (base->on_mouseenter || base->on_mouseleave))
    mouseevent_motion(renderer, event, base, event_flags);
}

Event *event_create(void)
{
  return (Event *)SDL_calloc(1, sizeof(Event_internal));
}