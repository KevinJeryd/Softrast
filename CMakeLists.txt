cmake_minimum_required(VERSION 3.23)
project(softrast)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(softrast)

target_sources(softrast PRIVATE 
                src/main.cpp
                src/renderer.cpp
                src/objParser.cpp
                )

target_include_directories(softrast PRIVATE include)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# SDL3
include(FetchContent)
FetchContent_Declare(
    SDL3
    URL https://github.com/libsdl-org/SDL/releases/download/release-3.2.8/SDL3-3.2.8.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(SDL3)

if(TARGET SDL3::SDL3main)
    target_link_libraries(softrast PRIVATE SDL3::SDL3main)
endif()
    target_link_libraries(softrast PRIVATE SDL3::SDL3)
