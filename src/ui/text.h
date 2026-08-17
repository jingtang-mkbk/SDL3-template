#ifndef TEXT_H
#define TEXT_H

#include "event/event.h"
#include "manager/manager.h"
#include <SDL3_ttf/SDL_ttf.h>

// clang-format off
typedef enum UI_TextAlign
{
  TextAlign_None,         /* w h不需要定义                                  TopCenter                       */ 
  TextAlign_TopLeft,      /*                                TopLeft ┌──────────────────────┐ TopRight       */
  TextAlign_TopCenter,    /*                                        |                      |                */
  TextAlign_TopRight,     /*                                        |                      |                */
  TextAlign_CenterLeft,   /*                                        |                      |                */
  TextAlign_Center,       /*                             CenterLeft |        Center        | CenterRight    */
  TextAlign_CenterRight,  /*                                        |                      |                */
  TextAlign_BottomLeft,   /*                                        |                      |                */
  TextAlign_BottomCenter, /*                                        |                      |                */
  TextAlign_BottomRight,  /*                             BottomLeft └──────────────────────┘ BottomRight    */ 
} UI_TextAlign;              /*                                               BottomCenter                       */
// clang-format on
typedef struct UI_Text
{
    Event base;
    char *text;
    char *path;
    float font_size;
    SDL_Color color;
    UI_TextAlign textalign;
} UI_Text;

// Create
UI_Text *UI_Text_CreateWithProp(SDL_FRect rect, char *text, char *path, float fontSize,
                                SDL_Color color, UI_TextAlign textalign);
// Render
void UI_Text_Render(SDL_Renderer *renderer, UI_Text *text);
// Deinit
void UI_Text_Deinit(void);

#endif /* TEXT_H */