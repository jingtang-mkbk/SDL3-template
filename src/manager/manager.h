#ifndef MANAGER_H
#define MANAGER_H

// clang-format off
#define MANAGER_FLAG_FONT  0x01
#define MANAGER_FLAG_IMAGE 0x02
#define MANAGER_FLAG_MUSIC 0x04
// clang-format on

#include "data.h"
#include "manager/font_manager.h"
#include "manager/music_manager.h"
#include "manager/texture_manager.h"

typedef struct Managers
{
    const FontManager *font_manager;
    const TextureManager *texture_manager;
    const MusicManager *music_manager;
} Managers;

typedef struct Manager
{
    void (*init)(Uint8 flags);
    void (*deinit)(void);
    Managers *(*get_managers)(void);
} Manager;

extern const Manager manager;

#endif /* MANAGER_H */