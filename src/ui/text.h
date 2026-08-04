#ifndef TEXT_H
#define TEXT_H

#include <SDL3_ttf/SDL_ttf.h>
#include "event/event.h"
#include "manager/manager.h"

// clang-format off
typedef enum TextAlign
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
} TextAlign;              /*                                               BottomCenter                       */
// clang-format on
typedef struct Text
{
  Event base;
  char *text;
  char *path;
  float font_size;
  SDL_Color color;
  SDL_Texture *texture;
  TextAlign textalign;
} Text;

void text_init(Text *text);
void text_init_multiple(Text *texts, int count);
void text_render(SDL_Renderer *renderer, Text *text);
void text_render_multiple(SDL_Renderer *renderer, Text *texts, int count);
void text_deinit(void);

#endif /* TEXT_H */