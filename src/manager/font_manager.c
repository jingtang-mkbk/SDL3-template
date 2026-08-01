#include "font_manager.h"

static FontEntry *font_map = NULL; /* string → TTF_Font* hash table */

static FontEntry *get_font_map()
{
  if (!font_map)
  {
    font_map = NULL; /* Initialize the hash table */
    SDL_Log("The font needs to be loaded before the font mapping table can be obtained.");
  }
  return font_map;
}

static bool init()
{
  if (TTF_Init() == -1)
  {
    SDL_Log("Failed to initialize SDL_ttf: %s", SDL_GetError());
    return false;
  }
  return true;
}

static void deinit()
{
  /* 释放所有已加载的字体 */
  for (int i = 0; i < shlen(font_map); i++)
    TTF_CloseFont(font_map[i].value);
  shfree(font_map);
  font_map = NULL;

  TTF_Quit();
}

static void load(const char *font_path)
{
  if (font_path == NULL)
    return;

  /* 如果已经加载过，跳过 */
  if (shget(font_map, font_path))
    return;

  TTF_Font *font = TTF_OpenFont(font_path, 12);
  if (!font)
  {
    SDL_Log("Failed to open font '%s': %s", font_path, SDL_GetError());
    return;
  }
  shput(font_map, font_path, font);
}

static void load_multiple(const char **font_paths, int count)
{
  for (int i = 0; i < count; i++)
    load(font_paths[i]);
}

static TTF_Font *get(const char *font_path)
{
  if (font_path == NULL)
    return NULL;
  return shget(font_map, font_path);
}

const FontManager font_manager = {
    .init = init,
    .deinit = deinit,
    .load = load,
    .load_multiple = load_multiple,
    .get = get,
};