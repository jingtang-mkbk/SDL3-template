#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include "data.h"

typedef struct Color
{
    Uint8 r, g, b;
} Color;

/* 从 "#RRGGBB" 解析颜色 */
Color parse_color(const char *hex);

#endif /* UTILS_H */
