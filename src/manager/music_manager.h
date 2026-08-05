#ifndef MUSIC_MANAGER_H
#define MUSIC_MANAGER_H

#include <SDL3_mixer/SDL_mixer.h>
#include "data.h"

#define MUSIC_BASE_PATH "assets/music/"

typedef struct MusicManager
{
  bool (*init)(void);
  void (*deinit)(void);
  void (*play)(const char *filepath);
  void (*pause)(const char *filepath);
} MusicManager;

extern const MusicManager music_manager;

#endif /* MUSIC_MANAGER_H */
