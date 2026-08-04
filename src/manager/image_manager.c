#include "image_manager.h"

static ImageEntry *image_map = NULL; /* string → SDL_Texture* hash table */

static ImageEntry *get_image_map()
{
  if (!image_map)
  {
    image_map = NULL; /* Initialize the hash table */
    SDL_Log("The image needs to be loaded before the image mapping table can be obtained.");
  }
  return image_map;
}

static void init() {}

static void deinit()
{
  for (int i = 0; i < shlen(image_map); i++)
    SDL_DestroyTexture(image_map[i].value);
  shfree(image_map);
  image_map = NULL;
}

static void load(SDL_Renderer *renderer, const char *image_path)
{
  if (image_path == NULL)
    return;

  /* 如果已经加载过，跳过 */
  if (shget(image_map, image_path))
    return;

  char full[100];
  SDL_snprintf(full, sizeof(full), "%s%s", IMAGE_BASE_PATH, image_path);
  SDL_Texture *texture = IMG_LoadTexture(renderer, full);
  if (!texture)
  {
    SDL_Log("Failed to load image '%s': %s", full, SDL_GetError());
    return;
  }
  shput(image_map, image_path, texture);
}

static SDL_Texture *get(SDL_Renderer *renderer, const char *image_path)
{
  if (image_path == NULL)
    return NULL;
  SDL_Texture *texture = shget(image_map, image_path);
  if (!texture)
  {
    /* 未加载则先加载，再取 */
    load(renderer, image_path);
    texture = shget(image_map, image_path);
  }
  return texture;
}

const ImageManager image_manager = {
    .init = init,
    .deinit = deinit,
    .get = get,
};