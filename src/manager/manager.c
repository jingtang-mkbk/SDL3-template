#include "manager.h"

static Managers *context = NULL;
static const Managers *get_managers(void)
{
    if (!context) {
        context = (Managers *)SDL_calloc(1, sizeof(Managers));
        if (!context) {
            SDL_Log("Failed to allocate memory for Manager instance.");
            return NULL;
        }
    }
    return context;
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
        mgr->texture_manager = &texture_manager;
        mgr->texture_manager->init();
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
        if (mgr->texture_manager)
            mgr->texture_manager->deinit();
        if (mgr->music_manager)
            mgr->music_manager->deinit();
        SDL_free(mgr);
        context = NULL;
    }
}

const Manager manager = {
    .init = init,
    .deinit = deinit,
    .get_managers = get_managers,
};