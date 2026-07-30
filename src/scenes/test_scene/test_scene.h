#ifndef __TEST_SCENE_H_
#define __TEST_SCENE_H_
#include <SDL3/SDL.h>
#include "data.h"
#include "utils/utils.h"

typedef struct cJSON cJSON;

typedef struct
{
  char text[32];
  Color color;
  float x, y;
} BackButton;

/* Scene-local data */
typedef struct TestSceneData
{
  BackButton back;
  float text_scale;
  float text_w, text_h;
  float parent_x, parent_y; /* RECT container offset */
  int hover_row, hover_col; /* mouse hover */
  int grid_cols;            /* from JSON, first row count */
  cJSON *children;          /* JSON array pointer */
} TestSceneData;

extern const Scene test_scene;

#endif /* __TEST_SCENE_H_ */
