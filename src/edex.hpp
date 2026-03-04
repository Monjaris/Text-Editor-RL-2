#pragma once
#include "utils.h"
#include "Tab.hpp"

struct Editor
{
    std::vector<Tab*> v_tabs;
    uint active = 0;

    void Reload();  // Start() also calls Reload(), that's why forward declared
    void Start();
    void Input();
    void Loop();
    void Render();
    void Cleanup();

    void new_tab() {
        v_tabs.push_back(new Tab());  // was emplace_back() — which pushed nullptr
        this->active = v_tabs.size()-1;
    }

    bool goto_tab(uint n) {
        if (v_tabs.empty() || (n > v_tabs.size()-1)) {
            log "Can't go to tab[" << n << "], out of bounds!\n";
            return false;
        }
        this->active = n;
        return true;
    }
    bool next_tab() {
        if (this->active == v_tabs.size()-1) {
            if (v_tabs[active]->cursor.flex >= Cursor::Flex::DYNAMIC) {
                log "Can't go to next tab, defaulting to first tab\n";
                return goto_tab(0);
            } else {
                log "Can't go to next tab, already at last\n";
                return false;
            }
        }
        return goto_tab(this->active + 1);
    }
    bool prev_tab() {
        if (this->active == 0) {
            if (v_tabs[active]->cursor.flex >= Cursor::Flex::DYNAMIC) {
                log "Can't go to previous tab, defaulting to last tab\n";
                return goto_tab(v_tabs.size()-1);
            } else {
                log "Can't go to previous tab, already at first\n";
                return false;
            }
        }
        return goto_tab(this->active - 1);
    }

};

