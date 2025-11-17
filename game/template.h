#pragma once

#include <raylib.h>
#include <raymath.h>
#include <assert.h>

#include "../include/fu_std.h"

// Anything inside this struct will be kept when hotreloading
typedef struct GameMemory {
	unsigned int frameCounter;

	Vector2 position;
} GameMemory;

FU_EXPORT void game_init_window();
FU_EXPORT void game_init();
FU_EXPORT bool game_update();
FU_EXPORT void game_shutdown();
FU_EXPORT void game_shutdown_window();
FU_EXPORT void* game_memory();
FU_EXPORT size_t game_memory_size();
FU_EXPORT void game_hotreloaded(void* memory);
FU_EXPORT bool game_should_restart();