#include "render.h"
#include "utils/utils.h"

Render_Element *Render_find_by_id(Render_Element *arr, const char *id)
{
    for (int i = 0; i < arrlen(arr); i++) {
        if (arr[i].id && SDL_strcmp(arr[i].id, id) == 0)
            return &arr[i];
        if (arr[i].children) {
            Render_Element *found = Render_find_by_id(arr[i].children, id);
            if (found)
                return found;
        }
    }
    return NULL;
}

void Render_set_callback(Render_Element *arr, const char *id, void (*click)(SDL_Event *, void *),
                         void *userdata)
{
    Render_Element *el = Render_find_by_id(arr, id);
    if (!el || !el->element)
        return;
    Event *base = (Event *)el->element;
    base->click = click;
    base->event_userdata = userdata;
}

void Render_set_callbacks(Render_Element *arr, const CallbackBinding *bindings, int count)
{
    for (int i = 0; i < count; i++)
        Render_set_callback(arr, bindings[i].id, bindings[i].click, bindings[i].userdata);
}

Render_Element *Render_json2element(SDL_Renderer *renderer, const char *filepath)
{
    Render_Element *arr = NULL;
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
        cJSON *type_json = cJSON_GetObjectItem(comp, "type");
        if (!cJSON_IsString(type_json))
            continue;

        Render_Element re = { 0 };
        cJSON *id_json = cJSON_GetObjectItem(comp, "id");
        if (cJSON_IsString(id_json))
            re.id = SDL_strdup(id_json->valuestring);

        if (SDL_strcmp(type_json->valuestring, "TEXT") == 0) {
            re.type = Render_Type_Text;
            cJSON *t = cJSON_GetObjectItem(comp, "text");
            cJSON *fs = cJSON_GetObjectItem(comp, "font_size");
            cJSON *color = cJSON_GetObjectItem(comp, "color");
            char *text_str = cJSON_IsString(t) ? SDL_strdup(t->valuestring) : NULL;
            UI_Text *text = UI_Text_CreateWithProp(
                (SDL_FRect){ 0, 0, 0, 0 }, text_str, NULL, /* 字体路径默认 */
                cJSON_IsNumber(fs) ? (float)fs->valuedouble : 0.0f,
                cJSON_IsString(color) ? parse_color(color->valuestring) : (SDL_Color){ 0, 0, 0, 0 },
                TextAlign_None);
            if (!text)
                SDL_free(text_str); /* CreateWithProp 失败（无 text 或字体缺失）时避免泄漏 */
            re.element = text;
        } else if (SDL_strcmp(type_json->valuestring, "IMAGE") == 0) {
            re.type = Render_Type_Image;
            cJSON *p = cJSON_GetObjectItem(comp, "path");
            if (cJSON_IsString(p)) {
                /* 深拷贝：cJSON_Delete 会释放 valuestring，而 deinit 时由 Render_deinit 释放 */
                char *path = SDL_strdup(p->valuestring);
                re.element = UI_Image_CreateWithProp(renderer, path, (SDL_FRect){ 0, 0, 0, 0 });
                if (!re.element)
                    SDL_free(path); /* 创建失败时避免泄漏 */
            }
        }

        if (re.element) {
            Event *base = (Event *)re.element;
            /* 先尝试 base.rect 嵌套格式（缺失项保留 UI_*_Create 的默认值，
               IMAGE 由 UI_Image_CreateWithProp 自动填充纹理尺寸） */
            cJSON *x = cJSON_GetObjectItem(comp, "x");
            cJSON *y = cJSON_GetObjectItem(comp, "y");
            cJSON *w = cJSON_GetObjectItem(comp, "w");
            cJSON *h = cJSON_GetObjectItem(comp, "h");
            if (cJSON_IsNumber(x))
                base->rect.x = (float)x->valuedouble;
            if (cJSON_IsNumber(y))
                base->rect.y = (float)y->valuedouble;
            if (cJSON_IsNumber(w))
                base->rect.w = (float)w->valuedouble;
            if (cJSON_IsNumber(h))
                base->rect.h = (float)h->valuedouble;
        }

        arrput(arr, re);
    }

    cJSON_Delete(root);
    SDL_free(buf);
    return arr;
}

void Render_init(SDL_Renderer *renderer, Render_Element *arr)
{
    for (int i = 0; i < arrlen(arr); i++) {
        switch (arr[i].type) {
        case Render_Type_Text:
            /* 文本在 UI_Text_CreateWithProp 时已设置字体与尺寸 */
            break;
        case Render_Type_Image:
            /* 图片在 UI_Image_CreateWithProp 时已加载纹理 */
            break;
        }

        if (arr[i].children)
            Render_init(renderer, arr[i].children);
    }
}

void Render_render(SDL_Renderer *renderer, Render_Element *arr)
{
    for (int i = 0; i < arrlen(arr); i++) {
        switch (arr[i].type) {
        case Render_Type_Text:
            UI_Text *text = (UI_Text *)arr[i].element;
            UI_Text_Render(renderer, text);
            if (arr[i].children)
                Render_render(renderer, arr[i].children);
            break;
        case Render_Type_Image:
            UI_Image *img = (UI_Image *)arr[i].element;
            UI_Image_Render(renderer, img);
            if (arr[i].children)
                Render_render(renderer, arr[i].children);
            break;
        case Render_Type_Button:
            break;
        default:
            break;
        }
    }
}

void Render_deinit(Render_Element *arr)
{
    if (!arr)
        return;

    for (int i = 0; i < arrlen(arr); i++) {
        SDL_free(arr[i].id); /* SDL_strdup 分配的 id */
        if (arr[i].children)
            Render_deinit(arr[i].children);

        switch (arr[i].type) {
        case Render_Type_Text:
            UI_Text *text = (UI_Text *)arr[i].element;
            if (text) {
                SDL_free(text->text); /* SDL_strdup 分配的文本；对象由 UI_Text_Deinit 统一释放 */
            }
            break;
        case Render_Type_Image:
            UI_Image *img = (UI_Image *)arr[i].element;
            if (img) {
                SDL_free(
                    img->path); /* SDL_strdup 分配的路径；img 对象由 UI_Image_Deinit 统一释放 */
            }
            break;
        default:
            SDL_free(arr[i].element);
            break;
        }
    }

    arrfree(arr);

    UI_Image_Deinit();
    UI_Text_Deinit();
}