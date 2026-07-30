#include "image.h"
#include <SDL3_image/SDL_image.h>
#include "stb_ds.h"

struct Image
{
  SDL_Texture *texture;
  int w, h;
};

void init_sprite(SpriteEntry **sprites, SDL_Renderer *renderer,
                 const char *name, const char *filepath,
                 float x, float y, float w, float h)
{
  SDL_Surface *surface = IMG_Load(filepath);
  if (!surface)
  {
    SDL_Log("init_sprite: failed to load '%s': %s", filepath, SDL_GetError());
    return;
  }

  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
  if (!texture)
  {
    SDL_Log("init_sprite: failed to create texture from '%s': %s", filepath, SDL_GetError());
    SDL_DestroySurface(surface);
    return;
  }

  Image *img = (Image *)SDL_calloc(1, sizeof(Image));
  if (!img)
  {
    SDL_DestroyTexture(texture);
    SDL_DestroySurface(surface);
    return;
  }

  img->texture = texture;
  img->w = surface->w;
  img->h = surface->h;

  SDL_Log("init_sprite: '%s' loaded (%dx%d), display %gx%g",
          name, img->w, img->h, w, h);

  SDL_DestroySurface(surface);

  Sprite s = {img, x, y, w, h};
  shput(*sprites, name, s);
}

void deinit_sprites(SpriteEntry **sprites)
{
  if (!*sprites)
    return;
  for (int i = 0; i < shlen(*sprites); i++)
  {
    Image *img = (*sprites)[i].value.img;
    if (img)
    {
      if (img->texture)
        SDL_DestroyTexture(img->texture);
      SDL_free(img);
    }
  }
  shfree(*sprites);
  *sprites = NULL;
}

Sprite *get_sprite(SpriteEntry **sprites, const char *name)
{
  if (!*sprites)
    return NULL;
  ptrdiff_t idx = shgeti(*sprites, name);
  if (idx < 0)
    return NULL;
  return &(*sprites)[idx].value;
}

void render_sprite(SDL_Renderer *renderer, SpriteEntry **sprites, const char *name)
{
  Sprite *s = get_sprite(sprites, name);
  if (!s || !s->img || !s->img->texture)
    return;
  SDL_FRect dst = {s->x, s->y, s->w, s->h};
  SDL_RenderTexture(renderer, s->img->texture, NULL, &dst);
}
