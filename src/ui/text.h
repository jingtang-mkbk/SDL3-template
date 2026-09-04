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
    Node node;
    char *text;           // 文本内容
    char *path;           // 文本font
    SDL_Texture *texture; // 纹理
    float font_size;      // 字体大小
    SDL_Color color;      // 字体颜色
} UI_Text;

typedef struct Text
{
    UI_Text *(*create)(SDL_FRect rect, char *txt, char *path, float fontSize, UI_TextAlign textalign);
    UI_Text *(*createWithClick)(SDL_FRect rect, char *txt, char *path, float fontSize, UI_TextAlign textalign, void *userdata, void *callback);
    UI_Text *(*createWithMultiEvent)(SDL_FRect rect, char *txt, char *path, float fontSize, UI_TextAlign textalign, void *userdata, Event_Userevent arr[], int count);
    void (*render)(SDL_Renderer *renderer, UI_Text *text);
    void (*deinit)(UI_Text *text);
    void (*setAlpha)(UI_Text *text, float alpha);
    void (*setColor)(UI_Text *text, char *color);
} Text;

extern const Text text;

#endif /* TEXT_H */