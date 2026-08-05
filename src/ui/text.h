#ifndef TEXT_H
#define TEXT_H

#include <SDL3_ttf/SDL_ttf.h>
#include "event/event.h"
#include "manager/manager.h"

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

void UI_text_init(UI_Text *text);
void UI_text_render(SDL_Renderer *renderer, UI_Text *text);
void UI_text_deinit(void);
UI_Text *UI_text_create(void);

#endif /* TEXT_H */