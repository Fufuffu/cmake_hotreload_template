project(HotreloadMain)

add_executable(HotreloadMain ../include/fu_std.h main.c)

target_include_directories(HotreloadMain PRIVATE ${CMAKE_SOURCE_DIR}/include)

if(WIN32)
    target_compile_definitions(HotreloadMain PRIVATE WIN32_LEAN_AND_MEAN)
    target_link_options(HotreloadMain PRIVATE /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup)
endif()

target_link_libraries(HotreloadMain PRIVATE gamelib)

if(UNIX)
    target_link_libraries(HotreloadMain PRIVATE ${CMAKE_DL_LIBS})
endif()

get_target_property(RAYLIB_TYPE raylib TYPE)
if(RAYLIB_TYPE STREQUAL "SHARED_LIBRARY")
    add_custom_command(TARGET HotreloadMain POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:raylib>
            $<TARGET_FILE_DIR:HotreloadMain>
    )
endif()
