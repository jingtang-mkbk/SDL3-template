#include "music.h"
#include <SDL3_mixer/SDL_mixer.h>

static MIX_Mixer *mixer = NULL;
static MIX_Audio *audio = NULL;
static MIX_Track *track = NULL;

bool music_init(const char *filepath)
{
  if (!MIX_Init())
  {
    SDL_Log("music_init: MIX_Init failed: %s", SDL_GetError());
    return false;
  }

  mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
  if (!mixer)
  {
    SDL_Log("music_init: MIX_CreateMixerDevice failed: %s", SDL_GetError());
    return false;
  }

  audio = MIX_LoadAudio(mixer, filepath, false);
  if (!audio)
  {
    SDL_Log("music_init: failed to load '%s': %s", filepath, SDL_GetError());
    MIX_DestroyMixer(mixer);
    mixer = NULL;
    return false;
  }

  track = MIX_CreateTrack(mixer);
  if (!track)
  {
    SDL_Log("music_init: MIX_CreateTrack failed: %s", SDL_GetError());
    MIX_DestroyAudio(audio);
    audio = NULL;
    MIX_DestroyMixer(mixer);
    mixer = NULL;
    return false;
  }

  if (!MIX_SetTrackAudio(track, audio))
  {
    SDL_Log("music_init: MIX_SetTrackAudio failed: %s", SDL_GetError());
    MIX_DestroyTrack(track);
    track = NULL;
    MIX_DestroyAudio(audio);
    audio = NULL;
    MIX_DestroyMixer(mixer);
    mixer = NULL;
    return false;
  }

  SDL_Log("music_init: loaded '%s'", filepath);
  return true;
}

void music_deinit(void)
{
  if (track)
  {
    MIX_DestroyTrack(track);
    track = NULL;
  }
  if (audio)
  {
    MIX_DestroyAudio(audio);
    audio = NULL;
  }
  if (mixer)
  {
    MIX_DestroyMixer(mixer);
    mixer = NULL;
  }
  MIX_Quit();
}

void music_play(void)
{
  if (!track)
    return;

  SDL_PropertiesID props = SDL_CreateProperties();
  SDL_SetNumberProperty(props, MIX_PROP_PLAY_LOOPS_NUMBER, -1);
  MIX_PlayTrack(track, props);
  SDL_DestroyProperties(props);
}

void music_pause(void)
{
  if (track)
    MIX_PauseTrack(track);
}
