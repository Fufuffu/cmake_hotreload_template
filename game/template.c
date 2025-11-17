#include "template.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

GameMemory* gMem;

void game_init_window() {
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Hotreload");
	SetTargetFPS(60);
	SetExitKey(KEY_NULL);
}

void game_init() {
	gMem = (GameMemory*)malloc(sizeof(GameMemory));
	assert(gMem && "RAM? :(");

	gMem->frameCounter = 0;
	gMem->position = (Vector2){ SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
}

bool game_update() {
	gMem->frameCounter++;

	if (IsKeyDown(KEY_W)) {
		gMem->position.y--;
	}
	if (IsKeyDown(KEY_S)) {
		gMem->position.y++;
	}
	if (IsKeyDown(KEY_A)) {
		gMem->position.x--;
	}
	if (IsKeyDown(KEY_D)) {
		gMem->position.x++;
	}

	BeginDrawing();
	{
		ClearBackground(SKYBLUE);

		// Try changing the color!
		DrawRectangleV(gMem->position, (Vector2) { 10.0f, 10.0f }, RED);

		DrawText(TextFormat("Frame: %u", gMem->frameCounter), SCREEN_WIDTH / 2, 10, 20, BLACK);
		DrawFPS(10, 10);
	}
	EndDrawing();

	return !WindowShouldClose();
}

void game_shutdown() {
	// Cleanup: unload any textures, etc...
}

void game_shutdown_window() {
	CloseWindow();
}

void* game_memory() {
	return gMem;
}

size_t game_memory_size() {
	return sizeof(GameMemory);
}

void game_hotreloaded(void* memory) {
	gMem = (GameMemory*)memory;
}

bool game_should_restart() {
	return IsKeyDown(KEY_R);
}