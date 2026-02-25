#pragma once
#include <raylib.h>
#include <iostream>
#include <vector>
#include <functional>
#include <fstream>
#include <filesystem>
#include <charconv>
#include <cassert>
#include <limits.h>
#include <unistd.h>

#define CSEGMENTS 5000
#define RSEGMENTS 250
#define Color3(r, g, b)  (Color){r, g, b, 255}

#if defined (__linux__) || defined (__APPLE__)
    #define EDEX_DEFAULT_BUFFER_FONT "/usr/share/fonts/TTF/JetBrainsMono-Regular.ttf"
    #define EDEX_DEFAULT_UI_FONT "/usr/share/fonts/TTF/JetBrainsMono-Regular.ttf"
//! TODO: implement font paths for Windows
#elif defined (_WIN32)
    #define EDEX_DEFAULT_BUFFER_FONT ""
    #define EDEX_DEFAULT_UI_FONT ""
#else
    #error Niche OS?
#endif 

using f32 = float;
using string = std::string;
using strview = std::string_view;
using Vec2 = Vector2;
using Rect = Rectangle;

