#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stddef.h>
#include "data.h"

#define container_of(ptr, type, member) \
  ((type *)((char *)(ptr) - offsetof(type, member)))

/* 从 "#RRGGBB" 或 "#RRGGBBAA" 解析颜色 */
SDL_Color parse_color(const char *hex);

/* 点 (mx, my) 是否落在矩形内 */
bool hit_in_rect(const SDL_FRect *rect, float mx, float my);

#endif /* UTILS_H */
