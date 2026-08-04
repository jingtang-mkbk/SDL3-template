#ifndef EVENT_H
#define EVENT_H

// clang-format off
#define EVENT_CLICK       0x01
#define EVENT_CLICK_RIGHT 0x02
#define EVENT_HOVER       0x04
#define EVENT_LEAVE       0x08
// clang-format on

#include "data.h"

typedef struct Event
{
  SDL_FRect rect;
  void (*on_click)(SDL_Event *event, void *userdata);
  void (*on_click_right)(SDL_Event *event, void *userdata);
  void (*on_mouseenter)(SDL_Event *event, void *userdata);
  void (*on_mouseleave)(SDL_Event *event, void *userdata);
  void *event_userdata;
  /* 内部状态：鼠标当前是否在元素内（用于检测 leave） */
  bool in_rect;
} Event;

void mouseevent_handle(SDL_Renderer *renderer, SDL_Event *event, Event *base, const Uint8 event_flags);

#endif // EVENT_H
