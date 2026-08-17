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
    void (*click)(SDL_Event *event, void *userdata);
    void (*click_right)(SDL_Event *event, void *userdata);
    void (*mouseenter)(SDL_Event *event, void *userdata);
    void (*mouseleave)(SDL_Event *event, void *userdata);
    void *event_userdata;
} Event;

void mouseevent_handle(SDL_Renderer *renderer, SDL_Event *event, Event *base,
                       const Uint8 event_flags);
Event *event_create(void);

#endif // EVENT_H
