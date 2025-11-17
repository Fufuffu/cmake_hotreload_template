project(HotreloadTemplate)

add_executable(HotreloadTemplate main.c)

target_include_directories(HotreloadTemplate PRIVATE ${CMAKE_SOURCE_DIR}/include)

if(WIN32)
    target_link_options(HotreloadTemplate PRIVATE /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup)
endif()

target_link_libraries(HotreloadTemplate PRIVATE gamelib)

if(UNIX)
    target_link_libraries(HotreloadTemplate PRIVATE ${CMAKE_DL_LIBS})
endif()