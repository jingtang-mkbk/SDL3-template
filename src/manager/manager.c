#include "manager.h"

static Managers *managers_ctx = NULL;
static Managers *get_managers(void)
{
    if (!managers_ctx) {
        managers_ctx = (Managers *)SDL_calloc(1, sizeof(Managers));
        if (!managers_ctx) {
            SDL_Log("Failed to allocate memory for Manager instance.");
            return NULL;
        }
    }
    return managers_ctx;
}

static void init(Uint8 flags)
{
    Managers *mgr = get_managers();
    if (!mgr)
        return;

    if (flags & MANAGER_FLAG_FONT) {
        mgr->font_manager = &font_manager;
        mgr->font_manager->init();
    }

    if (flags & MANAGER_FLAG_IMAGE) {
        mgr->image_manager = &image_manager;
        mgr->image_manager->init();
    }

    if (flags & MANAGER_FLAG_MUSIC) {
        mgr->music_manager = &music_manager;
        mgr->music_manager->init();
    }
}

static void deinit()
{
    Managers *mgr = get_managers();
    if (mgr) {
        if (mgr->font_manager)
            mgr->font_manager->deinit();
        if (mgr->image_manager)
            mgr->image_manager->deinit();
        if (mgr->music_manager)
            mgr->music_manager->deinit();
        SDL_free(mgr);
        managers_ctx = NULL;
    }
}

const Manager manager = {
    .init = init,
    .deinit = deinit,
    .get_managers = get_managers,
};