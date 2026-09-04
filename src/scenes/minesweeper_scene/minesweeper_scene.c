#include "minesweeper_scene.h"
#include "ui/ui.h"

#define GRID_COLS 10

/* 前向声明（辅助函数定义在文件后部） */
static SDL_Color mine_number_color(int n);
static UI_Text *make_text(const SDL_FRect *rect, const char *str, SDL_Color color);
static void free_text(UI_Text **t);
static UI_Text *make_content_text(const Cell *cell);
static UI_Text *make_flag_text(const Cell *cell);

static Cell *parsejson2cells(const char *filepath)
{
    Cell *cells = NULL;
    SDL_IOStream *io = SDL_IOFromFile(filepath, "r");
    if (!io)
        return NULL;

    Sint64 size = SDL_GetIOSize(io);
    char *buf = SDL_malloc(size + 1);
    SDL_ReadIO(io, buf, size);
    buf[size] = '\0';
    SDL_CloseIO(io);

    cJSON *root = cJSON_Parse(buf);
    if (!root) {
        SDL_free(buf);
        return NULL;
    }

    cJSON *components = cJSON_GetObjectItem(root, "components");
    if (!cJSON_IsArray(components)) {
        cJSON_Delete(root);
        SDL_free(buf);
        return NULL;
    }

    cJSON *comp;
    cJSON_ArrayForEach(comp, components)
    {
        cJSON *id = cJSON_GetObjectItem(comp, "id");
        if (!cJSON_IsString(id))
            continue;
        if (SDL_strcmp(id->valuestring, "1") == 0) {
            // todo
        } else if (SDL_strcmp(id->valuestring, "2") == 0) {
            cJSON *parent_x = cJSON_GetObjectItem(comp, "x");
            cJSON *parent_y = cJSON_GetObjectItem(comp, "y");
            cJSON *children = cJSON_GetObjectItem(comp, "children");
            cJSON *child;
            cJSON_ArrayForEach(child, children)
            {
                cJSON *child_x = cJSON_GetObjectItem(child, "x");
                cJSON *child_y = cJSON_GetObjectItem(child, "y");
                cJSON *w = cJSON_GetObjectItem(child, "w");
                cJSON *h = cJSON_GetObjectItem(child, "h");
                cJSON *color = cJSON_GetObjectItem(child, "background_color");
                cJSON *opacity = cJSON_GetObjectItem(child, "opacity");
                if (!cJSON_IsString(color))
                    continue;
                Cell cell = {
                    .rect = {
                        .x = (float)parent_x->valueint + (float)child_x->valueint,
                        .y = (float)parent_y->valueint + (float)child_y->valueint,
                        .w = (float)w->valueint,
                        .h = (float)h->valueint,
                    },
                    .color = parse_color("#AB705F"),
                    .status = MineType_Hidden,
                    .isMine = false,
                    .isHovered = false,
                    .opacity = opacity->valuedouble,
                };
                arrput(cells, cell);
            }
        }
    }

    cJSON_Delete(root);
    SDL_free(buf);
    return cells;
}

static void back_clicked(SDL_Event *event, void *userdata)
{
    AppState *state = (AppState *)userdata;
    state->switch_scene(state, "main_scene");
}

/* 根据坐标找到格子索引，未命中返回 -1 */
static int cell_index_at(MinesweeperSceneData *d, float mx, float my)
{
    for (int i = 0; i < arrlen(d->cells); i++) {
        if (hit_in_rect(&d->cells[i].rect, mx, my))
            return i;
    }
    return -1;
}

/* 翻开格子；空白格子递归翻开相邻空白区域 */
static void reveal_cell(MinesweeperSceneData *d, int index)
{
    if (index < 0 || index >= arrlen(d->cells))
        return;
    Cell *c = &d->cells[index];
    if (c->status != MineType_Hidden || c->isMine)
        return;
    c->status = MineType_Reveale;
    if (c->mine_count > 0) {
        c->text = make_content_text(c);
        return; /* 数字格子：只翻开自己 */
    }

    int rows = arrlen(d->cells) / GRID_COLS;
    int row = index / GRID_COLS, col = index % GRID_COLS;
    for (int dr = -1; dr <= 1; dr++) {
        for (int dc = -1; dc <= 1; dc++) {
            if (dr == 0 && dc == 0)
                continue;
            int nr = row + dr, nc = col + dc;
            if (nr < 0 || nr >= rows || nc < 0 || nc >= GRID_COLS)
                continue;
            reveal_cell(d, nr * GRID_COLS + nc);
        }
    }
}

/* 踩到雷：全部翻开并显示 Game Over */
static void trigger_game_over(AppState *state)
{
    MinesweeperSceneData *d = SCENE_DATA(state, MinesweeperSceneData);
    d->game_over = true;
    d->win = false;
    for (int i = 0; i < arrlen(d->cells); i++) {
        Cell *c = &d->cells[i];
        free_text(&c->text); /* 清掉可能插的旗 */
        c->status = MineType_Reveale;
        if (c->isMine || c->mine_count > 0)
            c->text = make_content_text(c);
    }
}

/* 所有非雷格子都已翻开则胜利 */
static bool check_win(MinesweeperSceneData *d)
{
    for (int i = 0; i < arrlen(d->cells); i++) {
        Cell *c = &d->cells[i];
        if (!c->isMine && c->status != MineType_Reveale)
            return false;
    }
    return true;
}

static void grid_clicked(SDL_Event *event, void *userdata)
{
    AppState *state = (AppState *)userdata;
    MinesweeperSceneData *d = SCENE_DATA(state, MinesweeperSceneData);
    if (d->game_over || d->win)
        return;

    /* click_event 已把坐标转换为渲染坐标 */
    int idx = cell_index_at(d, event->button.x, event->button.y);
    if (idx < 0)
        return;

    Cell *c = &d->cells[idx];
    if (c->status != MineType_Hidden)
        return; /* 已翻开或已插旗，忽略 */

    if (c->isMine) {
        trigger_game_over(state);
        return;
    }
    reveal_cell(d, idx);

    if (check_win(d))
        d->win = true;
}

static void grid_rightclicked(SDL_Event *event, void *userdata)
{
    AppState *state = (AppState *)userdata;
    MinesweeperSceneData *d = SCENE_DATA(state, MinesweeperSceneData);
    if (d->game_over || d->win)
        return;

    int idx = cell_index_at(d, event->button.x, event->button.y);
    if (idx < 0)
        return;

    Cell *c = &d->cells[idx];
    if (c->status == MineType_Reveale)
        return; /* 已翻开，不能插旗 */

    if (c->status == MineType_Hidden) {
        c->status = MineType_Flag;
        c->text = make_flag_text(c);
    } else /* MineType_Flag：再次右键取消插旗 */
    {
        c->status = MineType_Hidden;
        free_text(&c->text); /* 移除 F */
    }
}

static void grid_mouseenter(SDL_Event *event, void *userdata)
{
    AppState *state = (AppState *)userdata;
    MinesweeperSceneData *d = SCENE_DATA(state, MinesweeperSceneData);

    /* hover_event 已把坐标转换为渲染坐标，直接读取 */
    float mx = event->button.x;
    float my = event->button.y;

    /* 遍历所有格子：重置高亮并标记鼠标所在格子 */
    for (int i = 0; i < arrlen(d->cells); i++) {
        SDL_FRect *r = &d->cells[i].rect;
        d->cells[i].isHovered = hit_in_rect(r, mx, my);
    }
}

static void grid_mouseleave(SDL_Event *event, void *userdata)
{
    AppState *state = (AppState *)userdata;
    MinesweeperSceneData *d = SCENE_DATA(state, MinesweeperSceneData);

    /* 鼠标移出网格，清除所有格子高亮 */
    for (int i = 0; i < arrlen(d->cells); i++)
        d->cells[i].isHovered = false;
}

/* 经典扫雷数字配色：1蓝 2绿 3红 4深蓝 5深红 6青 7黑 8灰 */
static SDL_Color mine_number_color(int n)
{
    switch (n) {
    case 1:
        return (SDL_Color){ 0, 0, 255, 255 };
    case 2:
        return (SDL_Color){ 0, 128, 0, 255 };
    case 3:
        return (SDL_Color){ 255, 0, 0, 255 };
    case 4:
        return (SDL_Color){ 0, 0, 128, 255 };
    case 5:
        return (SDL_Color){ 128, 0, 0, 255 };
    case 6:
        return (SDL_Color){ 0, 128, 128, 255 };
    case 7:
        return (SDL_Color){ 0, 0, 0, 255 };
    default:
        return (SDL_Color){ 128, 128, 128, 255 }; /* 8 */
    }
}

/* 创建居中文本 */
static UI_Text *make_text(const SDL_FRect *rect, const char *str, SDL_Color color)
{
    return ui.getComps()->text->create(*rect, SDL_strdup(str), "MSYHBD.TTC", 40, TextAlign_Center);
}

static void free_text(UI_Text **t)
{
    if (*t) {
        SDL_free((*t)->text); /* strdup 的字符串；对象由 UI_Text_Deinit 统一释放 */
        *t = NULL;
    }
}

/* 翻开后显示的内容：雷 ● / 数字 */
static UI_Text *make_content_text(const Cell *cell)
{
    if (cell->isMine)
        return make_text(&cell->rect, "●", (SDL_Color){ 255, 80, 80, 255 });
    char buf[2] = { (char)('0' + cell->mine_count), '\0' };
    return make_text(&cell->rect, buf, mine_number_color(cell->mine_count));
}

/* 插旗显示的 F */
static UI_Text *make_flag_text(const Cell *cell)
{
    return make_text(&cell->rect, "F", (SDL_Color){ 255, 200, 0, 255 });
}

static void initMine(AppState *state)
{
    MinesweeperSceneData *d = SCENE_DATA(state, MinesweeperSceneData);
    d->cells = parsejson2cells("assets/json/minesweeper_scene.json");

    const int MINE_COUNT = 10;
    int total = arrlen(d->cells);
    int rows = total / GRID_COLS;

    /* 1. 随机布雷 */
    SDL_srand(0);
    int cell_count = 0;
    for (int i = 0; i < total; i++) {
        int index = SDL_rand(total);
        if (cell_count >= MINE_COUNT)
            break;
        if (!d->cells[index].isMine) {
            d->cells[index].isMine = true;
            cell_count++;
        }
    }

    /* 2. 统计每个格子 8 邻域内的雷数 */
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < GRID_COLS; c++) {
            Cell *cell = &d->cells[r * GRID_COLS + c];
            cell->mine_count = 0;
            if (cell->isMine)
                continue;

            for (int dr = -1; dr <= 1; dr++) {
                for (int dc = -1; dc <= 1; dc++) {
                    if (dr == 0 && dc == 0)
                        continue;
                    int nr = r + dr, nc = c + dc;
                    if (nr < 0 || nr >= rows || nc < 0 || nc >= GRID_COLS)
                        continue;
                    if (d->cells[nr * GRID_COLS + nc].isMine)
                        cell->mine_count++;
                }
            }
        }
    }

    /* 3. 文字不预创建：插旗时设 F，翻开时设 ●/数字 */
}

/* 重新开始游戏：清空棋盘并重新布雷 */
static void restart_game(AppState *state)
{
    MinesweeperSceneData *d = SCENE_DATA(state, MinesweeperSceneData);

    /* 清空旧棋盘 */
    for (int i = 0; i < arrlen(d->cells); i++)
        free_text(&d->cells[i].text);
    arrfree(d->cells);
    d->cells = NULL;

    d->game_over = false;
    d->win = false;

    /* 重新布雷 */
    initMine(state);
}

static void restart_clicked(SDL_Event *event, void *userdata)
{
    AppState *state = (AppState *)userdata;
    restart_game(state);
}

static void init(AppState *state)
{
    MinesweeperSceneData *d = (MinesweeperSceneData *)SDL_calloc(1, sizeof(MinesweeperSceneData));
    state->scene_data = d;

    // back
    d->back_tex = ui.getComps()->text->createWithClick((SDL_FRect){ 5, 5, 0, 0 }, "Back", NULL, 24, TextAlign_None, state, back_clicked);

    // restart（Back 下方）
    d->restart_tex = ui.getComps()->text->createWithClick((SDL_FRect){ 5, 40, 0, 0 }, "Restart", NULL, 24, TextAlign_None, state, restart_clicked);

    // 初始化雷
    initMine(state);

    // grid
    d->grid = Node_Create();
    // grid->rect = (SDL_FRect){ .x = 100, .y = 0, .w = 600, .h = 600 };
    // grid->event.click = grid_clicked;
    // grid->event.click_right = grid_rightclicked;
    // grid->event.mouseenter = grid_mouseenter;
    // grid->event.mouseleave = grid_mouseleave;
    // grid->event.userdata = state;
    // grid->event.userevent_count = 0;
    // grid->event.userevent_arr = NULL;
    Node_SetPosition(d->grid, 100, 0);
    Node_SetSize(d->grid, 600, 600);
    Node_SetMultiMouseEvent(d->grid, state, (Event_Userevent[]){
                                                { MOUSEEVENT_CLICK, grid_clicked },
                                                { MOUSEEVENT_CLICK_RIGHT, grid_rightclicked },
                                                { MOUSEEVENT_ENTER, grid_mouseenter },
                                                { MOUSEEVENT_LEAVE, grid_mouseleave },
                                            },
                            4);
    // d->grid = grid;

    // Game Over 文字（居中显示，游戏结束才渲染）
    int pw, ph;
    SDL_GetCurrentRenderOutputSize(state->renderer, &pw, &ph);
    d->game_over_text = ui.getComps()->text->create(
        (SDL_FRect){ 0, 0, (float)pw, (float)ph }, SDL_strdup("Game Over"),
        "MSYHBD.TTC", 60, TextAlign_Center);

    // You Win 文字（居中显示，胜利才渲染）
    d->win_text = ui.getComps()->text->create(
        (SDL_FRect){ 0, 0, (float)pw, (float)ph }, SDL_strdup("You Win"),
        "MSYHBD.TTC", 60, TextAlign_Center);

    /* 开启混合模式，否则 SDL_RenderFillRect 会忽略 alpha */
    SDL_SetRenderDrawBlendMode(state->renderer, SDL_BLENDMODE_BLEND);
}

static void event(AppState *state, SDL_Event *event)
{
    MinesweeperSceneData *d = SCENE_DATA(state, MinesweeperSceneData);

    if (event->type == SDL_EVENT_KEY_DOWN || event->type == SDL_EVENT_QUIT) {
        state->app_quit = SDL_APP_SUCCESS;
        return;
    }

    mouseevent(state->renderer, event, (Node *)d->back_tex);
    mouseevent(state->renderer, event, (Node *)d->restart_tex);
    mouseevent(state->renderer, event, (Node *)d->grid);
}

static void iterate(AppState *state)
{
    MinesweeperSceneData *d = SCENE_DATA(state, MinesweeperSceneData);

    SDL_SetRenderDrawColor(state->renderer, 20, 20, 30, 255);
    SDL_RenderClear(state->renderer);

    // Back / Restart
    ui.getComps()->text->render(state->renderer, d->back_tex);
    ui.getComps()->text->render(state->renderer, d->restart_tex);
    // grid
    for (int i = 0; i < arrlen(d->cells); i++) {
        Cell *c = &d->cells[i];
        SDL_FRect *rect = &c->rect;
        SDL_Color color = c->color;

        if (c->status == MineType_Reveale) {
            /* 已翻开：alpha 固定为 255*0.6，hover 不变色 */
            Uint8 a = (Uint8)(color.a * c->opacity);
            SDL_SetRenderDrawColor(state->renderer, color.r, color.g, color.b, a);
            SDL_RenderFillRect(state->renderer, rect);
            if (c->text)
                ui.getComps()->text->render(state->renderer, c->text);
        } else if (c->status == MineType_Flag) {
            Uint8 a = c->isHovered ? (Uint8)(color.a * c->opacity) : color.a;
            SDL_SetRenderDrawColor(state->renderer, color.r, color.g, color.b, a);
            SDL_RenderFillRect(state->renderer, rect);
            if (c->text)
                ui.getComps()->text->render(state->renderer, c->text); /* 显示 F */
        } else                                                         /* MineType_Hidden */
        {
            Uint8 a = c->isHovered ? (Uint8)(color.a * c->opacity) : color.a;
            SDL_SetRenderDrawColor(state->renderer, color.r, color.g, color.b, a);
            SDL_RenderFillRect(state->renderer, rect);
            /* 隐藏：不显示任何文字 */
        }
    }

    // 胜利 / 失败 居中显示
    if (d->win && d->win_text)
        ui.getComps()->text->render(state->renderer, d->win_text);
    else if (d->game_over && d->game_over_text)
        ui.getComps()->text->render(state->renderer, d->game_over_text);

    SDL_RenderPresent(state->renderer);
}

static void deinit(AppState *state)
{
    MinesweeperSceneData *d = SCENE_DATA(state, MinesweeperSceneData);

    /* back/restart/win/game_over/cell 文本对象已登记到 ui 全局注册表，退出时由 ui.deinit 统一释放；
       这里仅释放各场景自己 strdup 的字符串与裸 Node */
    for (int i = 0; i < arrlen(d->cells); i++)
        free_text(&d->cells[i].text);
    if (d->win_text)
        SDL_free(d->win_text->text); /* strdup 的字符串 */
    if (d->game_over_text)
        SDL_free(d->game_over_text->text); /* strdup 的字符串 */
    SDL_free(d->grid);
    SDL_free(state->scene_data);
    state->scene_data = NULL;
}

const Scene minesweeper_scene = {
    .init = init,
    .event = event,
    .iterate = iterate,
    .deinit = deinit,
};
