#ifndef IMAGE_H
#define IMAGE_H

#include "event/event.h"
#include "manager/manager.h"
#include <SDL3_image/SDL_image.h>
#include <stdbool.h>

typedef struct UI_Image
{
    Event base;
    char *path;
} UI_Image;

// Create
UI_Image *UI_Image_CreateWithProp(SDL_Renderer *renderer, char *path, SDL_FRect rect);
// Render
void UI_Image_Render(SDL_Renderer *renderer, UI_Image *img);
// Deinit
void UI_Image_Deinit(void);

void UI_Image_SetPosition(UI_Image *_img, float x, float y);
void UI_Image_SetSize(UI_Image *_img, float w, float h);
void UI_Image_SetAnchor(UI_Image *_img, float offsetX, float offsetY);
void UI_Image_SetAplha(UI_Image *_img, float alpha);
void UI_Image_SetRotate(UI_Image *_img, double deg);
void UI_Image_SetHidden(UI_Image *_img, bool hidden);
void UI_Image_SetUserdata(UI_Image *_img, void *userdata);
void UI_Image_SetClick(UI_Image *_img, void *callback);
void UI_Image_SetRightClick(UI_Image *_img, void *callback);
void UI_Image_SetMouseenter(UI_Image *_img, void *callback);
void UI_Image_SetMouseleave(UI_Image *_img, void *callback);

#endif // IMAGE_H