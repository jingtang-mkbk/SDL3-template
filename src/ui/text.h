#ifndef TEXT_H
#define TEXT_H

#include "event/mouse_event.h"
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
} UI_TextAlign;           /*                                               BottomCenter                     */
// clang-format on
typedef struct UI_Text
{
    UI_Event base;
    char *text;           // 文本内容
    char *path;           // 文本font
    SDL_Texture *texture; // 纹理
    float font_size;      // 字体大小
    SDL_Color color;      // 字体颜色
} UI_Text;

// Create
UI_Text *UI_Text_Create(SDL_FRect rect, char *txt, char *path, float fontSize, UI_TextAlign textalign);
UI_Text *UI_Text_CreateWithClick(SDL_FRect rect, char *txt, char *path, float fontSize, UI_TextAlign textalign, void *userdata, void *callback);
UI_Text *UI_Text_CreateWithMultiEvent(SDL_FRect rect, char *txt, char *path, float fontSize, UI_TextAlign textalign, void *userdata, Event_Userevent arr[], int count);
// Render
void UI_Text_Render(SDL_Renderer *renderer, UI_Text *text);
// Deinit
void UI_Text_Deinit(void);

void UI_Text_SetAplha(UI_Text *text, float alpha);
void UI_Text_SetColor(UI_Text *text, char *color);

#endif /* TEXT_H */