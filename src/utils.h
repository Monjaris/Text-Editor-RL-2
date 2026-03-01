#pragma once
#include "core.h"
#include "edex.hpp"


inline const char* ctos(char ch) {
    static thread_local char buf[2] = {0};
    buf[0] = ch;
    buf[1] = '\0';
    return buf;
}

template <typename T>
requires std::is_integral_v<T>
inline const char* itos(T x) {
    constexpr unsigned bufsize = std::numeric_limits<T>::
        digits10 + 2;  // add 2 for signed ints and '\0'
    static thread_local char buf[bufsize];
    auto res = std::to_chars(buf, buf+bufsize, x);
    if (res.ec == std::errc()) {
        *res.ptr = '\0';
        return buf;
    }
    return __func__;  // as fallback handler
}

template <typename T>
requires std::is_floating_point_v<T>
inline const char* ftos(T x) {
    constexpr unsigned bufsize =  std::numeric_limits<T>::max_digits10
    + std::numeric_limits<T>::max_exponent10 + 5;  // -,+, .,e, '\0'
    static thread_local char buf[bufsize];
    auto res = std::to_chars(buf, buf+bufsize, x);
    if (res.ec == std::errc()) {
        *res.ptr = '\0';
        return buf;
    }
    return __func__;  // as fallback handler
}


inline bool is_printable(char c) {
    return (c>=' ' && c<='~');
}

/// pass false to @arg `include_ws` if you want to exclude it from symbol filtering
inline bool is_symbol(char c, bool include_ws=true) {
    return
        ((include_ws? (c>=' '):(c>' ')) && (c<'0'))
        || ((c>'9') && (c<'A'))
        || ((c>'Z') && (c<'a'))
        || ((c>'z') && (c<='~'));
}


inline bool HasKeyPressing(int key) {
    return IsKeyPressed(key) || IsKeyPressedRepeat(key);
}
template <bool char_mode>
inline bool HasKeyPressing(int& out_key) {
    static_assert(char_mode,
        "This function only suppports to be called with char_mode=true"
    );

    int ch = GetCharPressed();
    if (ch > 0 && is_printable(ch)) {
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
    const Font& font, char ch, Vec2 pos, Color fclr, f32 fsize=32, f32 spacing=1
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


enum class CharKind { WS=1, TAB, SYM, ABC, INT, UNKNOWN };
inline CharKind getCharKind(char c) {
    using enum CharKind;
    if (!is_printable(c)) {
        log "Can't get char kinf of '"<<c<<"', its not printable\n";
    }
    switch (c) {
        case ' ':  return WS;
        break;
        case '\t': return TAB;
        break;
    }
    if (is_symbol(c, false)) {  // WS handled already, but codes can move around
        return SYM;
    } else if (std::isalpha(c)) {
        return ABC;
    } else if (std::isdigit(c)) {
        return INT;
    } else {
        log "Couldn't get CharKind of " << c << "\n";
        return UNKNOWN;
    }
    return CharKind(0);
}

