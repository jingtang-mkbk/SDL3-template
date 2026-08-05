#ifndef MINESWEEPER_SCENE_H
#define MINESWEEPER_SCENE_H

#include <stdbool.h>
#include "data.h"
#include "cJSON.h"
#include "utils/utils.h"
#include "ui/text.h"
#include "manager/manager.h"

typedef enum MineType
{
  MineType_Flag,
  MineType_Hidden,
  MineType_Reveale
} MineType;

typedef struct Cell
{
  SDL_FRect rect;
  SDL_Color color;
  MineType status;
  bool isMine;
  bool isHovered;
  float opacity;
  Uint8 mine_count; /* 8 邻域内的雷数 */
  UI_Text *text;    /* 当前显示的文本：插旗时 F，翻开时 雷●/数字 */
} Cell;
typedef struct MinesweeperSceneData
{
  UI_Text *back_tex;
  UI_Text *restart_tex;
  Cell *cells;
  Event *grid;
  bool game_over;
  bool win;
  UI_Text *game_over_text;
  UI_Text *win_text;
} MinesweeperSceneData;

extern const Scene minesweeper_scene;

#endif /* __MINESWEEPER_SCENE_H_ */
