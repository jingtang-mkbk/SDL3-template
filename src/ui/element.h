#ifndef ELEMENT_H
#define ELEMENT_H

#define ELEMENT_CLICK 0x01
#define ELEMENT_HOVER 0x02

#include "data.h"

typedef struct Element
{
  SDL_FRect rect;
  void (*on_click)(void *userdata);
  void (*on_hover)(void *userdata);
  void *event_userdata;
} Element;

bool element_handle_mouseevent(Element *base, float mx, float my, const Uint8 event_flags);

#endif // ELEMENT_H