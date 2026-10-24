cmake_minimum_required(VERSION 3.15)
project(Egoist LANGUAGES CXX RC)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DUNICODE -D_UNICODE -D_WIN32_WINNT=0x0A00 -DWINVER=0x0A00)
include_directories(${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/resources)

add_executable(Egoist WIN32
    src/main.cpp
    src/about.cpp
    src/settings_window.cpp
    src/Ui.cpp
    src/Image.cpp
    src/Lang.cpp
    src/Config.cpp
    src/Blockers.cpp
    src/egoist.rc
)

target_link_libraries(Egoist PRIVATE
    gdiplus comctl32 dwmapi uxtheme shell32 advapi32 ole32 user32 gdi32)

if(MSVC)
    target_compile_options(Egoist PRIVATE /utf-8 /W3)
    target_link_options(Egoist PRIVATE /MANIFEST:NO)
else()
    target_link_options(Egoist PRIVATE -static -static-libgcc -static-libstdc++)
endif()
