#include "edex.hpp"

TextBuffer buffer;


struct Cursor {
    Color color = {240, 240, 240, 150};
    f32 roundness = 0.0;

    uint bufx = 0;
    uint bufy = 0;

    Vec2 pos = {0, 0};

    Vec2 size = {16, 34};
    f32 step_width = 22;
    f32 line_height = step_width * buffer.ln_wh_k;
    bool keep_size_as_font_size = false;
    uint saved_max_bufx = 0;

    enum class Flex { RIGID=1, DYNAMIC, ELASTIC };
    Flex flex = Flex::ELASTIC;

    /// @brief Get x,y,w,h components as rectangle
    Rect rect() { return {pos.x, pos.y, size.x, size.y}; }

    /// @brief Just return line number (.bufy)
    decltype(bufy) ln() { return this->bufy; }

    // Check if cursor is within window bounds
    bool inside_r() { return pos.x < win_w - 2*size.x; }
    bool inside_l() { return pos.x > 0; }
    bool inside_u() { return pos.y > 0; }
    bool inside_d() { return pos.y <= win_h - 2*size.y; }

    // Check if (ch)aracter exists (safe buffer bounds)
    bool chexist_r() {
        return bufy < buffer.line_count
        && bufx < buffer.str(bufy).size();
    }
    bool chexist_l() {
        return bufy < buffer.line_count
        && bufx > 0 && bufx - 1 < buffer.str(bufy).size();
    }
    bool chexist_u() {
        return bufy > 0 &&  bufy - 1 < buffer.line_count
        && bufx < buffer.str(bufy - 1).size();
    }
    bool chexist_d() {
        return bufy + 1 < buffer.line_count
        && bufx < buffer.str(bufy + 1).size();
    }

    /// @brief convert positions to buffer indexes
    uint xtoi(f32 x) {
        return static_cast<uint>(x / step_width);
    }
    uint ytoi(f32 y) {
        return static_cast<uint>(y / line_height);
    }

    /// @brief pass integer x to step cursor x times, -x to step cursor -x times
    void step(int count = 1) {
        auto total = count;

        int new_bufx = static_cast<int>(bufx) + count;
        if (new_bufx < 0) new_bufx = 0;
        if ((uint)new_bufx > buffer.str(bufy).size()) new_bufx = buffer.str(bufy).size();
        bufx = new_bufx;

        log "Cursor stepped " << total << " to "
            << (total>0? "right":"left") << "\n";
    }

    void gotoln(uint line_no) {
        const uint& maxlines = buffer.line_count;
        if (line_no >= buffer.line_count) {
            log "Can't go line number above" <<
            maxlines << "!\n"; return;
        }
        if (line_no == maxlines) {
            line_no = maxlines;
        }

        log "Went to line \033[4;1m" << line_no << "\033[0;37m(+1)\n";
        this->bufy = line_no;

        // Clamp bufx to new line's length
        if (bufx > buffer.str(bufy).size()) {
            bufx = buffer.str(bufy).size();
        }
        // Make it apply saved max bufx if flexible
        if (flex >= Flex::ELASTIC) {
            bufx = clamp(saved_max_bufx, 0u, buffer.str(bufy).size());
        }
    }

    /// goes to end of the line, @returns passed char count
    uint line_end() {
        uint old_bufx = bufx;
        bufx = buffer.str(bufy).size();
        return bufx - old_bufx;
    }
    /// goes to start of the line, @returns step count
    uint line_home() {
        uint old_bufx = bufx;
        bufx = 0;
        return old_bufx;
    }

    // delete previous char or next char if you pass true
    void delete_c(bool next_c = false) {
        auto& bufdata = buffer.vec();  /// Literally data(vector) of the `TextBuffer buffer`
        if (next_c) {
            if (bufx < (buffer.str(bufy).size())) {
                buffer.remove(bufx, bufy);
            }
            else if (flex>=Flex::DYNAMIC && (ln()<buffer.line_count-1)) {
                log "Delete(next_c): it's start of the line, delete + concat line\n";
                const string del_line = bufdata[bufy+1];
                // gotoln(ln());
                this->line_home();
                this->step(bufdata[bufy].size());
                bufdata[bufy].append(del_line);
                bufdata.erase(bufdata.begin()+bufy+1);
            }
        }
        // delete character back(backspace key action)
        else {
            if (bufx > 0) {
                if (buffer.remove(bufx - 1, bufy)) {
                    this->step(-1);
                }
            } else if (flex>=Flex::DYNAMIC && ln()) {
                if (bufdata[bufy].empty()) {
                    log "Delete: line is empty, deleting line entry!\n";
                    bufdata.erase(bufdata.begin()+bufy);
                    gotoln(ln()-1);
                    this->line_end();
                }
                else if (flex >= Flex::DYNAMIC) {
                    log "Delete: it's start of the line, delete + concat line\n";
                    const string del_line = bufdata[bufy];
                    bufdata[bufy-1].append(del_line);
                    bufdata.erase(bufdata.begin()+bufy);
                    gotoln(ln()-1);
                    this->line_end();
                    this->step(-del_line.size());
                }
            }
        }
    }

    // delete previous word or next word if you pass true
    void delete_word(bool next_w=false) {
        auto& bufdata = buffer.vec();  /// Literally data(vector) of the `TextBuffer buffer`
        if (!next_w && !bufx) {
            log "delete_word(back): No char left to delete!\n";
            return;
        } else if (next_w && (bufx >= bufdata[bufy].size())) {
            log "delete_word(front): No char left to delete!\n";
            return;
        }
        if (!next_w) {
            log "Deleting word back\n";
            CharKind ck = getCharKind(buffer(bufx-1, bufy, false),false);
            while ((bufx>0) && (getCharKind(buffer(bufx-1,bufy,false),false)==ck)) {
                log ":: " << buffer(bufx-1, bufy, false)
                << "  :  " << bufx << "\n";
                delete_c();
            }
        } else {  // next_w == true
            log "Deleting word front\n";
            CharKind ck = getCharKind(buffer(bufx, bufy, false));
            while ((bufx>0) && (getCharKind(buffer(bufx,bufy,false))==ck)) {
                delete_c(true);
            }
        }
    }


    void reload(bool first=false) {
        logx (first? "Loading" : "Reloading") << " cursor.." logxe;
        this->bufy = 0;
        this->bufx = 0;
    }

    void start() {
        reload(true);
    }

#pragma region LOOP
    void loop() {
        if (keep_size_as_font_size) {
            size.x = step_width;
            size.y = line_height;
        }
        step_width = buffer.char_width;
        line_height = buffer.font_size * buffer.ln_wh_k;

        // set cursor's position according to line number(cursor.line)
        pos.x = buffer.linepad.padding + (bufx * step_width);
        pos.y = (bufy * line_height);

        if (bufx > saved_max_bufx) {
            saved_max_bufx = bufx;
        }

        /// this variable is used via hasKeyPressing<true>(wkey) only
        int wkey; // well because that function writes into it, then operates
        /// @brief Writing text
        if (hasKeyPressing<true>(wkey)) {
            buffer.insert(wkey, this->bufx, this->bufy);
            this->step(+1);
        }

        /// @brief move keys
        if (hasKeyPressing(KEY_RIGHT) && inside_r()) {
            if (chexist_r()) {
                this->step();
            } else if (flex >= Flex::DYNAMIC && ln()+1 < buffer.line_count) {
                log "Wrapped to next line\n";  // there is no text in cursor's right & next line exists
                this->gotoln(ln()+1);
                this->line_home();
            }
        } else if (hasKeyPressing(KEY_LEFT)) {
            if (chexist_l()) {
                this->step(-1);
            } else if (flex >= Flex::DYNAMIC && ln()) {
                log "Wrapped to previous line\n";  // there is no text in cursor's left & previous line exists
                this->gotoln(ln()-1);
                this->line_end();
            }
        } else if (hasKeyPressing(KEY_UP)) {
            if (inside_u()) {
                this->gotoln(ln()-1);
            } else if (flex >= Flex::DYNAMIC) {  // there is no text above cursor
                log "Unwrapped to EOL\n";
                this->line_home();
                this->saved_max_bufx = 0;
            }
        } else if (hasKeyPressing(KEY_DOWN)) {
            if (buffer.line_count > ln()+1) {
                this->gotoln(ln()+1);
            } else if (flex >= Flex::DYNAMIC) {  // there is no text below cursor
                log "Unwrapped to BOL\n";
                this->line_end();
                this->saved_max_bufx = 0;
            }
        }

        /// @brief deleting text
        if (hasKeyPressing(KEY_BACKSPACE)) {
            if (isCtrlDown(true, true)) {  // CTRL+<-
                this->delete_word();
            } else if (not isModKeyDown()) {
                this->delete_c();
            }
        }  // Delete characters in reversed direction
        else if (hasKeyPressing(KEY_DELETE)) {
            if (isCtrlDown(true ,true)) {  // CTRL+DEL
                this->delete_word(true);
            } else if (not isModKeyDown()) {
                this->delete_c(true);
            }
        }

        /// @brief line breaking with enter key
        if (hasKeyPressing(KEY_ENTER)) {
            log "Line break: (" << bufx << ", " << bufy << ")\n";
            string& line = buffer.str(ln());
            string leftstr = line.substr(bufx);
            buffer.str(ln()).erase(bufx);
            buffer.vec().insert(buffer.vec().begin()+ln()+1, leftstr);
            bufx = 0; ++bufy;
        }

        if (hasKeyPressing(KEY_TAB)) {
            this->step(buffer.insert_tab(bufx, bufy));
        }


        if (hasKeyPressing(KEY_Q)) {
            if (isCtrlDown(true, true)) {  // Ctrl+Q
                log "Exiting EDEX\n";
                std::exit(EXIT_SUCCESS);
            }
        }
        else if (hasKeyPressing(KEY_S)) {
            if (isCtrlDown(true, true)) {  // CTRL+S
                TRY(buffer.save(stdfs::current_path().string()),
                    log "Saving buffer couldn't done\n";
                );
            }
        }
        else if (IsKeyPressed(KEY_O)) {
            if (isCtrlDown(true, true)) {   // CTRL+O
                TRY(buffer.load("file.edex.cc"),
                    log "Loading file  couldn't done!\n";
                );
                reload();
            }
        }
    }
    void draw() {
        DrawRectangleRounded(
            rect(), roundness, RSEGMENTS, color
        );
    }
};


Cursor cursor;


#pragma region CALLERS

void setup()
{
    buffer.start();
    cursor.start();
}

void update()
{
    buffer.loop(cursor.pos, cursor.ln());
    cursor.loop();
}

void render()
{
    buffer.draw();
    cursor.draw();
}


