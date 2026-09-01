#ifndef IMAGE_H
#define IMAGE_H

#include "event/mouse_event.h"
#include "manager/manager.h"
#include <SDL3_image/SDL_image.h>

typedef struct UI_Image
{
    UI_Event base;
    char *path;
    SDL_Texture *texture;
} UI_Image;

// Create
UI_Image *UI_Image_Create(SDL_Renderer *renderer, char *path, SDL_FRect rect);
UI_Image *UI_Image_CreateWithClick(SDL_Renderer *renderer, char *path, SDL_FRect rect, void *userdata, void *callback);
UI_Image *UI_Image_CreateWithMultiEvent(SDL_Renderer *renderer, char *path, SDL_FRect rect, void *userdata, Event_Userevent arr[], int count);
// Render
void UI_Image_Render(SDL_Renderer *renderer, UI_Image *img);
// Deinit
void UI_Image_Deinit(void);

void UI_Image_SetAplha(UI_Image *img, float alpha);

#endif // IMAGE_H