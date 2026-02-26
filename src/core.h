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
#define log std::cerr<<
#define logx std::cerr << "\033[1;34m" <<
#define logxe "\033[0m\n"
#define TRY(stmt, errmsg)  do{if(!stmt){log errmsg;}}while(0);

#define EDEX_DEFAULT_BUFFER_FONT "assets/fonts/JetBrainsMono/JetBrainsMono-Regular.ttf"
#define EDEX_DEFAULT_UI_FONT "assets/fonts/InterVariable/InterVariable.ttf"

using f32 = float;
using string = std::string;
using strview = std::string_view;
using Vec2 = Vector2;
using Rect = Rectangle;

