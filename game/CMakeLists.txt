project(gamelib)

set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

if(CMAKE_BUILD_TYPE MATCHES "Debug")
    set(GAMELIB_TYPE SHARED)
    set(USE_SHARED_RAYLIB ON)
else()
    set(GAMELIB_TYPE STATIC)
    set(USE_SHARED_RAYLIB OFF)
endif()

add_library(gamelib ${GAMELIB_TYPE}
    template.c
    template.h
    fu_std_impl.c
    ../include/fu_std.h
)

set_target_properties(gamelib PROPERTIES POSITION_INDEPENDENT_CODE ON)

get_target_property(TARGET_TYPE gamelib TYPE)
if(TARGET_TYPE STREQUAL SHARED_LIBRARY)
    target_compile_definitions(gamelib
        PRIVATE $<BUILD_INTERFACE:BUILD_LIBTYPE_SHARED>
        INTERFACE $<INSTALL_INTERFACE:USE_LIBTYPE_SHARED>
    )
endif()

find_package(raylib 5.5 QUIET)
if(NOT raylib_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 5.5
    )
    set(BUILD_SHARED_LIBS ${USE_SHARED_RAYLIB})
    FetchContent_MakeAvailable(raylib)
endif()

target_link_libraries(gamelib PUBLIC raylib)

if(UNIX AND NOT APPLE)
    target_link_libraries(gamelib PUBLIC m ${CMAKE_DL_LIBS} pthread)
elseif(APPLE)
    target_link_libraries(gamelib PUBLIC m pthread)
endif()

if(NOT raylib_FOUND AND USE_SHARED_RAYLIB)
    add_custom_command(TARGET gamelib POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:raylib>
            $<TARGET_FILE_DIR:gamelib>
    )
endif()
