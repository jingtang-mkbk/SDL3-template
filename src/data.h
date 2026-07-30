#ifndef DATA_H
#define DATA_H
#include <SDL3/SDL.h>
#include "stb_ds.h"

/* Convenience: cast state->scene_data to the scene's local type */
#define SCENE_DATA(state, Type) ((Type *)(state)->scene_data)

/* Forward declarations */
typedef struct AppState AppState;
typedef struct Image Image;

/* A sprite on screen: image + position + display size */
typedef struct Sprite
{
  Image *img;
  float x, y;
  float w, h;
} Sprite;

/* Hash map entry: string name → Sprite */
typedef struct SpriteEntry
{
  char *key;
  Sprite value;
} SpriteEntry;

/* Scene interface — like a virtual table, one per scene */
typedef struct Scene
{
  void (*init)(AppState *);
  void (*event)(AppState *, SDL_Event *);
  void (*iterate)(AppState *);
  void (*deinit)(AppState *);
} Scene;

/* Hash map entry: string key → Scene pointer */
typedef struct SceneEntry
{
  char *key;
  const Scene *value;
} SceneEntry;

typedef struct AppState
{
  /* SDL resources (lifetime of the app) */
  SDL_Window *window;
  SDL_Renderer *renderer;
  int app_quit;

  /* Scene routing */
  SceneEntry *scene_map; /* string → Scene* hash table */
  const Scene *current_scene;
  const Scene *next_scene;
  void *scene_data;
  void (*switch_scene)(AppState *, const char *);
} AppState;

#endif // DATA_H