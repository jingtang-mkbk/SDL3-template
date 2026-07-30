#ifndef ENGINE_MUSIC_H
#define ENGINE_MUSIC_H
#include <stdbool.h>

bool music_init(const char *filepath);
void music_deinit(void);
void music_play(void);
void music_pause(void);

#endif // ENGINE_MUSIC_H
