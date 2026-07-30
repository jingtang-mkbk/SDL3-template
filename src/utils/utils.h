#ifndef UTILS_H
#define UTILS_H

#include <SDL3/SDL.h>

typedef struct
{
    Uint8 r, g, b;
} Color;

/* 从 "#RRGGBB" 解析颜色 */
Color parse_color(const char *hex);

#endif /* UTILS_H */
