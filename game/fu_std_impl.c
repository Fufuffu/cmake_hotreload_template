// See: https://github.com/raysan5/raylib/discussions/1569#discussioncomment-428801

#if defined(_WIN32) || defined(_WIN64)
#ifndef NOGDI
#define NOGDI
#endif
#ifndef NOUSER
#define NOUSER
#endif
#endif
#define FU_IMPLEMENTATION
#include "../include/fu_std.h"
