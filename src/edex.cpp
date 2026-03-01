#include "edex.hpp"

TextBuffer buffer;


struct Cursor {
    Color color = {240, 240, 240, 150};
    f32 roundness = 0.0;

    uint bufx = 0;
    uint bufy = 0;

    Vec2 pos = {0, 0};

    bool keep_size_as_font_size = 0;
    f32 step_width = 22;
    f32 line_height = step_width * buffer.ln_wh_k;
    Vec2 size = {16, 34};

    enum class Flex { RIGID=1, DYNAMIC, ELASTIC };
    Flex flex = Flex::ELASTIC;
    Flex flex_previous = Flex(0);

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
                this->step(-1);
                TRY(
                    !buffer.remove(bufx, bufy),
                    "Couldn't remove!\n"
                );
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
            CharKind ck = getCharKind(buffer(bufx-1, bufy, false));
            while ((bufx>0) && (getCharKind(buffer(bufx-1,bufy,false))==ck)) {
                delete_c();
            }
        } else {  // next_w == true
            log "Deleting word front\n";
            CharKind ck = getCharKind(buffer(bufx+1, bufy, false));
            while ((bufx>0) && (getCharKind(buffer(bufx+1,bufy,false))==ck)) {
                delete_c(true);
            }
        }
    }


    void reload() {

    }
    void start() {
        reload();
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

        /// this variable is used via HasKeyPressing<true>(wkey) only
        int wkey; // well because that function writes into it, then operates

        /// @brief Writing text
        if (HasKeyPressing<true>(wkey)) {
            buffer.insert(wkey, this->bufx, this->bufy);
            this->step(+1);
        }

        /// @brief move keys
        if (HasKeyPressing(KEY_RIGHT) && inside_r()) {
            if (chexist_r()) {
                this->step();
            } else if (flex >= Flex::DYNAMIC && ln()+1 < buffer.line_count) {
                log "Wrapped to next line\n";  // there is no text in cursor's right & next line exists
                this->gotoln(ln()+1);
                this->line_home();
            }
        } else if (HasKeyPressing(KEY_LEFT)) {
            if (chexist_l()) {
                this->step(-1);
            } else if (flex >= Flex::DYNAMIC && ln()) {
                log "Wrapped to previous line\n";  // there is no text in cursor's left & previous line exists
                this->gotoln(ln()-1);
                this->line_end();
            }
        } else if (HasKeyPressing(KEY_UP)) {
            if (inside_u()) {
                this->gotoln(ln()-1);
            } else if (flex >= Flex::DYNAMIC) {  // there is no text above cursor
                log "Unwrapped to EOL\n";
                this->line_home();
            }
        } else if (HasKeyPressing(KEY_DOWN)) {
            if (buffer.line_count > ln()+1) {
                this->gotoln(ln()+1);
            } else if (flex >= Flex::DYNAMIC) {  // there is no text below cursor
                log "Unwrapped to BOL\n";
                this->line_end();
            }
        }

        /// @brief deleting text
        if (HasKeyPressing(KEY_BACKSPACE)) {
            this->delete_c();
        }  // Delete characters in reversed direction
        else if (HasKeyPressing(KEY_DELETE)) {
            this->delete_c(true);
        }

        /// @brief line breaking with enter key
        if (HasKeyPressing(KEY_ENTER)) {
            string& line = buffer.str(ln());
            string leftstr = line.substr(bufx);
            buffer.str(ln()).erase(bufx);
            buffer.vec().insert(buffer.vec().begin()+ln()+1, leftstr);
            bufx = 0; ++bufy;
        }

        if (HasKeyPressing(KEY_TAB)) {
            this->step(buffer.insert_tab(bufx, bufy));
        }

        /// CTRL-key bindings
        if (IsKeyDown(KEY_LEFT_CONTROL)) {
            if (HasKeyPressing(KEY_BACKSPACE)) {
                this->delete_word();
            }
            else if (IsKeyPressed(KEY_S)) {
                if (buffer.fsave(stdfs::current_path().string())) {
                    // saved successfully(logging handled by fsave())
                } else {
                    logx "Some file operation error occured, check out: "
                    __FILE__ << ":" << __LINE__
                    << logxe;
                }
            }
            else if (IsKeyPressed(KEY_O)) {
                // `TextBuffer::fload()` WILL BE IMPLEMENTED IN THE NEXT UPDATE!
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


