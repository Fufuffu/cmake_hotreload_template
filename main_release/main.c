#include "../game/template.h"

int main(int argc, char *argv[]) {
    game_init_window();
    game_init();

    while (true) {
        if (!game_update()) {
            break;
        }
    }

    game_shutdown();
    game_shutdown_window();

    return 0;
}
