#ifndef EVENTS_H
#define EVENTS_H

#include "keyboard_event.h"
#include "mouse_event.h"

typedef struct Event
{
    const MouseEventApi *mouse;
    const KeyboardEventApi *keyboard;
} Event;

extern const Event events;

#endif // EVENTS_H