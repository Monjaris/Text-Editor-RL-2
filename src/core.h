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
#define logxe "\033[0m\n"
#define TRY(stmt, errmsg)  do{if(!stmt){log errmsg;}}while(0);
#define CAST(type, value)  (static_cast<type>(value))
#define FEQ(flt1, flt2)  (std::fabsf(flt1 - flt2) < 0.01f)  // f1==f2 is unprecise

#define EDEX_DEFAULT_BUFFER_FONT "assets/fonts/JetBrainsMono/JetBrainsMono-Regular.ttf"
#define EDEX_DEFAULT_UI_FONT "assets/fonts/InterVariable/InterVariable.ttf"
#define EDEX_DEFAULT_BUFFER_NAME "file.edex.cc"
#define EDEX_FONT_SIZE_MIN 2

namespace stdfs = std::filesystem;

using f32 = float;
using string = std::string;
using strview = std::string_view;
using Vec2 = Vector2;
using Rect = Rectangle;

