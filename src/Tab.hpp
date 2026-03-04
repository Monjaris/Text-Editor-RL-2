#pragma once
#include "TextBuffer.hpp"
#include "Cursor.hpp"

struct Tab
{
    TextBuffer buffer;
    Cursor cursor;

    Tab ()
    : cursor(&buffer)
    {
        start();
    }

    void reload();
    void start();
    void input();
    void loop();
    void render();
    void cleanup();

};

inline void Tab::start()
{
    buffer.start();
    cursor.start();
    this->reload();
}

inline void Tab::reload()
{

}

inline void Tab::input()
{
    if (hasKeyPressing(KEY_S)) {
        log "Tab::input: KEY_S pressed; isCtrlDown=" << (isCtrlDown()?1:0) << "\n";
        if (isCtrlDown()) {  // CTRL+S
            TRY(buffer.save(buffer.cwd),
                log "Saving buffer couldn't done\n";
            );
        }
    }
    else if (hasKeyPressing(KEY_O)) {
        log "Tab::input: KEY_O pressed; isCtrlDown=" << (isCtrlDown()?1:0) << "\n";
        if (isCtrlDown()) {   // CTRL+O
            TRY(buffer.load("file.edex.cc"),
                log "Loading file  couldn't done!\n";
            );
            reload();
        }
    }
}

inline void Tab::loop()
{
    // process tab-specific input (save/load shortcuts, etc.)
    this->input();

    buffer.loop(cursor.pos, cursor.ln());
    cursor.loop();
}

inline void Tab::render()
{
    buffer.render();
    cursor.render();
}

inline void Tab::cleanup()
{

}
