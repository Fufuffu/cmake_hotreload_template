cmake_minimum_required(VERSION 3.29)

if(POLICY CMP0141)
    cmake_policy(SET CMP0141 NEW)
    if(MSVC)
        set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>")
    endif()
endif()

project(HotreloadTemplate LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(WIN32)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

if(MSVC)
    add_compile_options(/W3)
elseif(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra)
endif()

add_subdirectory(game)

if(CMAKE_BUILD_TYPE MATCHES "Debug")
    add_subdirectory(main)
else()
    add_subdirectory(main_release)
endif()