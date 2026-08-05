#include "music_manager.h"

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

static MusicEntry *music_map = NULL; /* string → {audio, track} hash table */
static MIX_Mixer *mixer = NULL;

static bool music_manager_init()
{
  return MIX_Init();
}

static void music_manager_deinit()
{
  for (int i = 0; i < shlen(music_map); i++)
  {
    if (music_map[i].value.track)
      MIX_DestroyTrack(music_map[i].value.track);
    if (music_map[i].value.audio)
      MIX_DestroyAudio(music_map[i].value.audio);
  }
  shfree(music_map);
  music_map = NULL;

  if (mixer)
  {
    MIX_DestroyMixer(mixer);
    mixer = NULL;
  }
  MIX_Quit();
}

static bool music_manager_load(const char *filepath)
{
  if (!filepath)
    return false;

  /* 已加载则跳过（shgetp 未找到时返回零值默认槽，须判断 track 是否真实存在） */
  MusicEntry *exist = shgetp(music_map, filepath);
  if (exist && exist->value.track)
    return true;

  if (!mixer)
  {
    mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
    if (!mixer)
    {
      SDL_Log("music_manager: MIX_CreateMixerDevice failed: %s", SDL_GetError());
      return false;
    }
  }

  char full[100];
  SDL_snprintf(full, sizeof(full), "%s%s", MUSIC_BASE_PATH, filepath);
  MIX_Audio *audio = MIX_LoadAudio(mixer, full, false);
  if (!audio)
  {
    SDL_Log("music_manager: failed to load '%s': %s", full, SDL_GetError());
    return false;
  }

  MIX_Track *track = MIX_CreateTrack(mixer);
  if (!track)
  {
    SDL_Log("music_manager: MIX_CreateTrack failed: %s", SDL_GetError());
    MIX_DestroyAudio(audio);
    return false;
  }

  if (!MIX_SetTrackAudio(track, audio))
  {
    SDL_Log("music_manager: MIX_SetTrackAudio failed: %s", SDL_GetError());
    MIX_DestroyTrack(track);
    MIX_DestroyAudio(audio);
    return false;
  }

  MusicData data = {.audio = audio, .track = track};
  shput(music_map, filepath, data);
  return true;
}

static void music_manager_play(const char *filepath)
{
  if (!filepath)
    return;
  MusicEntry *e = shgetp(music_map, filepath);
  if (!e || !e->value.track)
  {
    /* 未加载则先加载，再播放 */
    if (!music_manager_load(filepath))
      return;
    e = shgetp(music_map, filepath);
    if (!e || !e->value.track)
      return;
  }

  SDL_PropertiesID props = SDL_CreateProperties();
  SDL_SetNumberProperty(props, MIX_PROP_PLAY_LOOPS_NUMBER, -1);
  if (!MIX_PlayTrack(e->value.track, props))
    SDL_Log("music_manager: MIX_PlayTrack failed: %s", SDL_GetError());
  SDL_DestroyProperties(props);
}

static void music_manager_pause(const char *filepath)
{
  if (!filepath)
    return;
  MusicEntry *e = shgetp(music_map, filepath);
  if (e && e->value.track)
    MIX_PauseTrack(e->value.track);
}

const MusicManager music_manager = {
    .init = music_manager_init,
    .deinit = music_manager_deinit,
    .play = music_manager_play,
    .pause = music_manager_pause,
};
