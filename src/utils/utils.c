#include "utils.h"

SDL_Color parse_color(const char *hex)
{
    SDL_Color c = {0, 0, 0, 255};
    if (hex && hex[0] == '#')
    {
        unsigned int r, g, b, a = 255;
        int len = SDL_strlen(hex);
        if (len == 9)
            sscanf(hex + 1, "%2x%2x%2x%2x", &r, &g, &b, &a);
        else
            sscanf(hex + 1, "%2x%2x%2x", &r, &g, &b);
        c.r = (Uint8)r;
        c.g = (Uint8)g;
        c.b = (Uint8)b;
        c.a = (Uint8)a;
    }
    return c;
}

bool hit_in_rect(const SDL_FRect *rect, float mx, float my)
{
    return rect &&
           mx >= rect->x && mx <= rect->x + rect->w &&
           my >= rect->y && my <= rect->y + rect->h;
}
