#ifndef SCENE_H
#define SCENE_H

#include "stb_ds.h"
#include <SDL3/SDL.h>

#define SCENE_DATA(state, Type) ((Type *)(state)->scene_data)

typedef struct Scene
{
    void (*init)(void *);
    void (*event)(void *, SDL_Event *);
    void (*iterate)(void *);
    void (*deinit)(void *);
} Scene;

typedef struct SceneManager
{
    void (*init)(void *);
    void (*event)(void *, SDL_Event *);
    void (*iterate)(void *);
    void (*deinit)(void *);
    void (*add)(const char *name, const Scene *scene);
    void (*remove)(const char *name);
    void (*replace)(const char *name);
    const Scene *(*get)(const char *name);
} SceneManager;

extern const SceneManager sceneManager;

#endif // SCENE_H