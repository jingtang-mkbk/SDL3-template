#ifndef MINESWEEPER_SCENE_H
#define MINESWEEPER_SCENE_H

#include "data.h"
#include "cJSON.h"
#include "utils/utils.h"
#include "ui/text.h"

typedef struct cJSON cJSON;

/* Scene-local data */
typedef struct MinesweeperSceneData
{
  Text back_tex;
  float parent_x, parent_y; /* RECT container offset */
  int hover_row, hover_col; /* mouse hover */
  int grid_cols;            /* from JSON, first row count */
  cJSON *children;          /* JSON array pointer */
} MinesweeperSceneData;

extern const Scene minesweeper_scene;

#endif /* __MINESWEEPER_SCENE_H_ */
