#ifndef ENGINE_IMAGE_H
#define ENGINE_IMAGE_H
#include <SDL3/SDL.h>
#include "../../data.h"

/**
 * Opaque handle for a loaded image.
 */
typedef struct Image Image;

/**
 * Load an image, create a Sprite, and insert it into the sprite map.
 * @param sprites   Pointer to the sprite hash map (SpriteEntry*)
 * @param name      Key name for the sprite (e.g. "tiger")
 * @param filepath  Path to the image file
 * @param x, y      Initial screen position
 * @param w, h      Display width and height
 */
void init_sprite(SpriteEntry **sprites, SDL_Renderer *renderer,
                 const char *name, const char *filepath,
                 float x, float y, float w, float h);

/**
 * Destroy all sprites in the map and free the map.
 */
void deinit_sprites(SpriteEntry **sprites);

/**
 * Get a sprite by name from the map. Returns NULL if not found.
 */
Sprite *get_sprite(SpriteEntry **sprites, const char *name);

/**
 * Render a sprite by name from the map (no-op if not found).
 */
void render_sprite(SDL_Renderer *renderer, SpriteEntry **sprites, const char *name);

#endif // ENGINE_IMAGE_H
