#include "music_manager.h"

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

  /* 已加载则跳过 */
  MusicEntry *exist = music_map ? shgetp(music_map, filepath) : NULL;
  if (exist)
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

  MIX_Audio *audio = MIX_LoadAudio(mixer, filepath, false);
  if (!audio)
  {
    SDL_Log("music_manager: failed to load '%s': %s", filepath, SDL_GetError());
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

static void music_manager_load_multiple(const char **filepaths, int count)
{
  for (int i = 0; i < count; i++)
    music_manager_load(filepaths[i]);
}

static void music_manager_play(const char *filepath)
{
  if (!filepath)
    return;
  MusicEntry *e = shgetp(music_map, filepath);
  if (!e || !e->value.track)
    return;

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
    .load = music_manager_load,
    .load_multiple = music_manager_load_multiple,
    .play = music_manager_play,
    .pause = music_manager_pause,
};
