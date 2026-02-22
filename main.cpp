#include "edex.hpp"

int main()
{
    SetConfigFlags(
        FLAG_WINDOW_RESIZABLE
        | FLAG_WINDOW_HIGHDPI
    );
    InitWindow(init_win_w, init_win_h, "");
    if (is_fps_capped) SetTargetFPS(target_fps);

    std::cout << "\e[37m";  // Set terminal font color white
    setup();
    while (!WindowShouldClose())
    {
        fps = GetFPS();
        dt = GetFrameTime();
        win_w = GetScreenWidth();
        win_h = GetScreenHeight();
        update();
        BeginDrawing();
        ClearBackground(bg_color);
        SetWindowTitle(std::to_string(fps).c_str());
        render();
        EndDrawing();
    }

    std::cout << "\n\n\e[0m";  // Reset terminal font color
    CloseWindow();
}

