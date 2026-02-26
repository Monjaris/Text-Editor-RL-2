#pragma once
#include "core.h"
#include "edex.hpp"


inline bool printable(char c) {
    return (c>=' ' && c<='~');
}

inline const char* ctos(char ch) {
    static thread_local char buf[2] = {0};
    buf[0] = ch;
    buf[1] = '\0';
    return buf;
}

template <typename T>
requires std::is_integral_v<T>
inline const char* itos(T x) {
    constexpr unsigned int64_digits = 21;
    static thread_local char buf[int64_digits];
    auto res = std::to_chars(buf, buf+int64_digits, x);
    if (res.ec == std::errc()) {
        *res.ptr = '\0';
        return buf;
    }
    return __func__;
}

inline bool HasKeyPressing(int key) {
    return IsKeyPressed(key) || IsKeyPressedRepeat(key);
}
/// @overload for printable ascii characters with modifier support
template <bool char_mode>
inline bool HasKeyPressing(int& out_key) {
    static_assert(char_mode,
        "This function only designed to be called with char_mode=true"
    );
    
    // Use GetCharPressed which handles Shift, AltGr, etc.
    int ch = GetCharPressed();
    if (ch > 0 && printable(ch)) {
        out_key = ch;
        return true;
    }
    
    return false;
}


// Render text : C-STRING
inline void renderText(
    const Font& font, const char* txt, Vec2 pos, Color fclr, f32 fsize=32, f32 spacing=1
){
    DrawTextEx(
        font, txt, pos, fsize, spacing, fclr
    );
}
// Render text : UNSIGNED-CHAR
inline void renderText(
    const Font& font, u_char ch, Vec2 pos, Color fclr, f32 fsize=32, f32 spacing=1
){
    const char txt[] = {ch, '\0'};
    DrawTextEx(
        font, txt, pos, fsize, spacing, fclr
    );
}
// Render text : STD::STRING
inline void renderText(
    const Font& font, const string& txt, Vec2 pos, Color fclr, f32 fsize=32, f32 spacing=1
){
    const char* _txt = txt.c_str();
    DrawTextEx(
        font, _txt, pos, fsize, spacing, fclr
    );
}

