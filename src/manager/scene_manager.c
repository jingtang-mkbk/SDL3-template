#include "scene_manager.h"

typedef struct SceneEntry
{
    char *key;
    const Scene *value;
} SceneEntry;

typedef struct SceneData
{
    SceneEntry *scene_map;
    const Scene *current_scene;
    const Scene *next_scene;
    void *scene_data;
    void (*switch_scene)(void *state, const char *name);
} SceneData;

static SceneEntry *scene_map = NULL;

static void add(const char *name, const Scene *scene)
{
    if (!name || !scene)
        return;
    shput(scene_map, name, scene);
}

static void remove(const char *name)
{
    if (!name)
        return;
    shdel(scene_map, name);
}

static const Scene *get(const char *name)
{
    if (!name)
        return NULL;

    return shget(scene_map, name);
}

static void replace(const char *name)
{
    if (!name)
        return;
    Scene *scene = get(name);
}

const SceneManager sceneManger = {
    .add = add,
    .remove = remove,
    .get = get,
};