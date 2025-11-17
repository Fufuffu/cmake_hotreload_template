# CMake Hot Reload Template

A simple template for hot reloading code using CMake and Raylib
(should be easy enough to change to any other dependency with 
the same API style).

## Building

The simplest way to use this template is to open it up in an 
IDE like Visual Studio, it will automatically pickup all targets
and presets.

In case you want to run cmake manually, here are some guidelines:

### Debug (hot reload)

```bash
# Configure
cmake --preset x64-debug

# Build
cmake --build out/build/x64-debug

# Build just the library
cmake --build out/build/x64-debug --target gamelib
```

### Release (static build)

```bash
# Configure
cmake --preset x64-release

# Build
cmake --build out/build/x64-release
```

## Running

### Debug Mode
```bash
./out/build/x64-debug/main/main ../game/gamelib
```

### Release Mode
```bash
./out/build/x64-release/main_release/HotreloadTemplate
```

## General Workflow

1. Start the application in debug mode (usually attached to a debugger in case anything goes wrong)
2. Modify [game/template.c](game/template.c)
3. Rebuild: `cmake --build out/build/x64-debug --target gamelib`
4. Modifications should be instantly picked up by the app
