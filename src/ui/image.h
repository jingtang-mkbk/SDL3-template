#ifndef IMAGE_H
#define IMAGE_H

#include "event/mouse_event.h"
#include "manager/manager.h"
#include <SDL3_image/SDL_image.h>

typedef struct UI_Image
{
    Node node;
    char *path;
    SDL_Texture *texture;
} UI_Image;

typedef struct Image
{
    UI_Image *(*create)(SDL_Renderer *renderer, char *path, SDL_FRect rect);
    UI_Image *(*createWithClick)(SDL_Renderer *renderer, char *path, SDL_FRect rect, void *userdata, void *callback);
    UI_Image *(*createWithMultiEvent)(SDL_Renderer *renderer, char *path, SDL_FRect rect, void *userdata, Event_Userevent arr[], int count);
    void (*render)(SDL_Renderer *renderer, UI_Image *img);
    void (*deinit)(UI_Image *img);
    void (*setAlpha)(UI_Image *img, float alpha);
} Image;

extern const Image image;

#endif // IMAGE_H