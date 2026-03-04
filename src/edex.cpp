#include "edex.hpp"

void Editor::Start()
{
    // initial tab
    this->new_tab();
}

void Editor::Reload()
{
    v_tabs[active]->reload();
}

void Editor::Input()
{
    if (hasKeyPressing(KEY_Q)) {
        if (isCtrlDown()) {  // Ctrl+Q
            log "Exit requested (Ctrl+Q)\n";
            app_should_close = true;
        }
    }
    else if (hasKeyPressing(KEY_T)) {
        if (isCtrlDown()) {  // Ctrl+T
            this->new_tab();
        }
    }
    if (hasKeyPressing(KEY_TAB)) {
        if (isCtrlDown(true, false) && isShiftDown(false, true)) {  // Ctrl+Shift+Tab
            this->prev_tab();
        }
        else if (isCtrlDown()) {  // Ctrl+Tab
            this->next_tab();
        }
    }
}

void Editor::Loop()
{
    // for (auto& tab  : v_tabs) {
        // tab.loop();
    // }
    v_tabs[active]->loop();
    Input();
}

void Editor::Render()
{
    // for (auto& tab  : v_tabs) {
        // tab.render();
    // }
    v_tabs[active]->render();
}

void Editor::Cleanup()
{
    for (auto* tab : v_tabs) {
        tab->cleanup();
        delete tab;
    }
    v_tabs.clear();
}


