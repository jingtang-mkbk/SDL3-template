#include "texture_manager.h"

typedef struct TextureEntry
{
    char *key;
    SDL_Texture *value;
} TextureEntry;

static const char *ASSETS_PATH = "assets/";

static TextureEntry *texture_map = NULL; /* string → SDL_Texture* hash table */

static void init()
{
    /* stb_ds 字符串表默认只存 key 指针不拷贝（STBDS_SH_DEFAULT）：
       精灵帧用栈缓冲当 key 会悬垂，导致 shget 查不到。
       开启 strdup 模式让 shput 自动拷贝 key（须在任何插入之前调用）。 */
    stbds_sh_new_strdup(texture_map);
}

static void deinit()
{
    for (int i = 0; i < shlen(texture_map); i++) {
        SDL_DestroyTexture(texture_map[i].value);
    }
    shfree(texture_map);
    texture_map = NULL;
}

static SDL_Texture *get(const char *path)
{
    if (path == NULL || texture_map == NULL)
        return NULL;

    SDL_Texture *texture = shget(texture_map, path);
    return texture;
}

static SDL_Texture *load(SDL_Renderer *renderer, const char *path)
{
    if (path == NULL)
        return NULL;

    /* 如果已经加载过，直接返回缓存的贴图（场景切换时 UI_Deinit 只释放结构体、不销毁贴图） */
    SDL_Texture *cached = shget(texture_map, path);
    if (cached)
        return cached;

    char full[100];
    SDL_snprintf(full, sizeof(full), "%s%s", ASSETS_PATH, path);
    SDL_Texture *texture = IMG_LoadTexture(renderer, full);
    if (!texture) {
        SDL_Log("Failed to load texture '%s': %s", full, SDL_GetError());
        return NULL;
    }
    shput(texture_map, path, texture);
    return texture;
}

static void remove(const char *path)
{
    if (path == NULL || texture_map == NULL)
        return;

    SDL_Texture *texture = shget(texture_map, path);
    if (texture) {
        SDL_DestroyTexture(texture);
        shdel(texture_map, path);
    }
}

// 返回第一个精灵的 SDL_Texture*，后续精灵可以通过 get() 获取
// key 约定与 UI_Sprite_Render 一致：path + 三位帧号（不带 assets/ 前缀、不带扩展名）
static SDL_Texture *load_sprite(SDL_Renderer *renderer, const char *path, const Uint8 count, const char *suffix)
{
    if (path == NULL || count <= 0)
        return NULL;

    SDL_Texture *first = NULL;
    for (int i = 0; i < count; i++) {
        char key[100];
        SDL_snprintf(key, sizeof(key), "%s%03d", path, i);

        SDL_Texture *cached = shget(texture_map, key);
        if (cached) {
            if (i == 0) /* 首帧已缓存：直接复用，保证返回非 NULL */
                first = cached;
            continue; // 如果已经加载过，跳过
        }

        char full[100];
        const char *ext = suffix ? suffix : ".png"; /* 默认 png */
        SDL_snprintf(full, sizeof(full), "%s%s%03d%s", ASSETS_PATH, path, i, ext);
        SDL_Texture *tex = IMG_LoadTexture(renderer, full);
        if (!tex) {
            SDL_Log("Failed to load sprite '%s': %s", full, SDL_GetError());
            continue;
        }
        shput(texture_map, key, tex);
        if (i == 0)
            first = tex;
    }
    return first;
}

static void remove_sprite(const char *path, Uint8 count)
{
    for (int i = 0; i < count; i++) {
        char key[100];
        SDL_snprintf(key, sizeof(key), "%s%03d", path, i);
        remove(key);
    }
}

const TextureManager texture_manager = {
    .init = init,
    .deinit = deinit,
    .get = get,
    .load = load,
    .remove = remove,
    .load_sprite = load_sprite,
    .remove_sprite = remove_sprite,
};