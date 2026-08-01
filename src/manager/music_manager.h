#ifndef MUSIC_MANAGER_H
#define MUSIC_MANAGER_H

#include <SDL3_mixer/SDL_mixer.h>
#include "data.h"

typedef struct MusicData
{
  MIX_Audio *audio;
  MIX_Track *track;
} MusicData;

typedef struct MusicEntry
{
  char *key;
  MusicData value;
} MusicEntry;

typedef struct MusicManager
{
  bool (*init)(void);
  void (*deinit)(void);
  bool (*load)(const char *filepath);
  void (*load_multiple)(const char **filepaths, int count);
  void (*play)(const char *filepath);
  void (*pause)(const char *filepath);
} MusicManager;

extern const MusicManager music_manager;

#endif /* MUSIC_MANAGER_H */
