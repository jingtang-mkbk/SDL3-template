#include "element.h"

static bool hit_in_rect(SDL_FRect *rect, float mx, float my)
{
  return mx >= rect->x && mx <= rect->x + rect->w &&
         my >= rect->y && my <= rect->y + rect->h;
}

bool element_handle_mouseevent(Element *base, const float mx, const float my, const Uint8 event_flags)
{
  if (!base || !hit_in_rect(&base->rect, mx, my))
    return false;

  if (event_flags & ELEMENT_CLICK && base->on_click)
    base->on_click(base->event_userdata);
  if (event_flags & ELEMENT_HOVER && base->on_hover)
    base->on_hover(base->event_userdata);
  return true;
}