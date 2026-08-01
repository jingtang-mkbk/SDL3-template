#include "utils.h"

Color parse_color(const char *hex)
{
    Color c = {0, 0, 0};
    if (hex && hex[0] == '#')
    {
        unsigned int r, g, b;
        sscanf(hex + 1, "%2x%2x%2x", &r, &g, &b);
        c.r = (Uint8)r;
        c.g = (Uint8)g;
        c.b = (Uint8)b;
    }
    return c;
}
