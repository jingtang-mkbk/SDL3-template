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

  char full[100];
  SDL_snprintf(full, sizeof(full), "%s%s", FONT_BASE_PATH, font_path);
  TTF_Font *font = TTF_OpenFont(full, 12);
  if (!font)
  {
    SDL_Log("Failed to open font '%s': %s", full, SDL_GetError());
    return;
  }
  shput(font_map, font_path, font);
}

static TTF_Font *get(const char *font_path)
{
  if (font_path == NULL)
    return NULL;
  TTF_Font *font = shget(font_map, font_path);
  if (!font)
  {
    /* 未加载则先加载，再取 */
    load(font_path);
    font = shget(font_map, font_path);
  }
  return font;
}

const FontManager font_manager = {
    .init = init,
    .deinit = deinit,
    .get = get,
};