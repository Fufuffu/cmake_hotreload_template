#include <stdio.h>
#include <string.h>

#define FU_IMPLEMENTATION
#include "../include/fu_std.h"

#if defined(PLATFORM_WINDOWS)
#define DYN_LIB_EXT ".dll"
#elif defined(PLATFORM_MAC)
#define DYN_LIB_EXT ".dylib"
#else
#define DYN_LIB_EXT ".so"
#endif

static const char* given_dll_name = NULL;
static char path_buf[512];
static char versioned_path_buf[512];

typedef struct GameAPI {
	void (*init)(void);
	void (*init_window)(void);
	bool (*update)(void);
	void (*shutdown)(void);
	void (*shutdown_window)(void);
	void* (*memory)(void);
	size_t(*memory_size)(void);
	void (*hotreloaded)(void*);
	bool (*should_restart)(void);
	fuDynLibHandle lib;
	int64_t dll_time;
	int api_version;
} GameAPI;

#define LOAD_SYMBOL(api, lib, name, type) \
	do { \
		(api).name = (type)fu_dyn_lib_get_symbol_address(lib, "game_" #name); \
		if (!(api).name) { \
			fu_dyn_lib_free(lib); \
			fprintf(stderr, "DLL missing required symbol: game_%s\n", #name); \
			return false; \
		} \
	} while(0)

bool load_game_api(int api_version, GameAPI* out_api) {
	const int64_t dll_time = fu_file_get_last_write_time(path_buf);
	if (dll_time == 0) {
		fprintf(stderr, "Could not fetch last write date of %s\n", path_buf);
		return false;
	}

	snprintf(versioned_path_buf, sizeof(versioned_path_buf), "%s_%d" DYN_LIB_EXT, given_dll_name, api_version);

	if (!fu_file_copy(path_buf, versioned_path_buf)) {
		fprintf(stderr, "Failed to copy %s to %s\n", path_buf, versioned_path_buf);
		return false;
	}

	fuDynLibHandle lib = fu_dyn_lib_load(versioned_path_buf);
	if (!lib) {
		fprintf(stderr, "Failed loading DLL %s\n", versioned_path_buf);
		return false;
	}

	GameAPI api = { 0 };
	LOAD_SYMBOL(api, lib, init, void(*)(void));
	LOAD_SYMBOL(api, lib, init_window, void(*)(void));
	LOAD_SYMBOL(api, lib, update, bool(*)(void));
	LOAD_SYMBOL(api, lib, shutdown, void(*)(void));
	LOAD_SYMBOL(api, lib, shutdown_window, void(*)(void));
	LOAD_SYMBOL(api, lib, memory, void* (*)(void));
	LOAD_SYMBOL(api, lib, memory_size, size_t(*)(void));
	LOAD_SYMBOL(api, lib, hotreloaded, void(*)(void*));
	LOAD_SYMBOL(api, lib, should_restart, bool(*)(void));
	api.lib = lib;
	api.dll_time = dll_time;
	api.api_version = api_version;

	*out_api = api;
	return true;
}

void unload_game_api(GameAPI* api) {
	if (api->lib) {
		fu_dyn_lib_free(api->lib);
		api->lib = NULL;
	}

	snprintf(versioned_path_buf, sizeof(versioned_path_buf), "%s_%d" DYN_LIB_EXT, given_dll_name, api->api_version);
	if (!fu_file_delete(versioned_path_buf)) {
		fprintf(stderr, "Failed to remove %s copy\n", versioned_path_buf);
	}
}

int main(int argc, char* argv[]) {
	if (argc != 2) {
		fprintf(stderr, "Usage: main dll_name\n");
		return 1;
	}

	given_dll_name = argv[1];
	snprintf(path_buf, sizeof(path_buf), "%s" DYN_LIB_EXT, given_dll_name);

	int game_api_version = 0;
	GameAPI game_api;

	if (!load_game_api(game_api_version, &game_api)) {
		fprintf(stderr, "Failed to load API\n");
		return 1;
	}

	game_api.init_window();
	game_api.init();

	while (game_api.update()) {
		const int64_t dll_time = fu_file_get_last_write_time(path_buf);
		const bool dll_changed = dll_time != 0 && game_api.dll_time != dll_time;
		const bool should_restart = game_api.should_restart();

		if (dll_changed || should_restart) {
			game_api_version++;
			GameAPI new_api;
			if (!load_game_api(game_api_version, &new_api)) {
				continue;
			}

			if (should_restart || game_api.memory_size() != new_api.memory_size()) {
				game_api.shutdown();
				unload_game_api(&game_api);
				game_api = new_api;
				game_api.init();
			}
			else {
				void* memory = game_api.memory();
				// TODO: This might become a problem if there's some kind of static data
				// being pointed to within the dll and we just unload it
				unload_game_api(&game_api);
				game_api = new_api;
				game_api.hotreloaded(memory);
			}
		}
	}

	game_api.shutdown();
	game_api.shutdown_window();
	unload_game_api(&game_api);

	return 0;
}