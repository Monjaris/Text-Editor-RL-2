#pragma once
#include <raylib.h>
#include <iostream>
#include <vector>
#include <functional>
#include <fstream>
#include <filesystem>
#include <charconv>
#include <limits>
#include <cmath>
#include <cassert>

#ifdef __linux__
    #include <limits.h>
    #include <unistd.h>
#else
    #include <windows.h>
    using uint = uint32_t;
#endif

#define CSEGMENTS 5000
#define RSEGMENTS 250
#define Color3(r, g, b)  (Color){r, g, b, 255}
#define log std::cerr<<
#define logx std::cerr << "\033[1;34m" <<
#define logxe "\033[0;37m\n"
#define TRY(stmt, run)  do{if(!stmt){run}}while(0)
#define CAST(type, value)  (static_cast<type>(value))
#define FEQ(flt1, flt2)  (std::fabsf(flt1 - flt2) < 0.01f)  // f1==f2 is unprecise

#define EDEX_DEFAULT_BUFFER_FONT "/home/ziya/dev/gfx/edex/assets/fonts/JetBrainsMono/JetBrainsMono-Regular.ttf"
#define EDEX_DEFAULT_UI_FONT "assets/fonts/InterVariable/InterVariable.ttf"
#define EDEX_DEFAULT_BUFFER_NAME "file.edex.cc"
#define EDEX_FONT_SIZE_MIN 2

namespace stdfs = std::filesystem;

using f32 = float;
using string = std::string;
using strview = std::string_view;
using Vec2 = Vector2;
using Rect = Rectangle;


constexpr inline f32 init_win_w = 1280;
constexpr inline f32 init_win_h = 720;
inline f32 win_w = init_win_w;
inline f32 win_h = init_win_h;
inline Color bg_color = {0, 0, 0, 255};
inline int target_fps = 60;
inline int is_fps_capped = false;
inline f32 fps, dt = 0;
inline Font font_buf;
inline Font font_ui;
// Application-level requested-close flag. Set to true to request a clean shutdown
inline bool app_should_close = false;

