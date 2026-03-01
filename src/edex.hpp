#pragma once
#include "utils.h"


constexpr inline f32 init_win_w = 1280;
constexpr inline f32 init_win_h = 720;
inline f32 win_w = init_win_w;
inline f32 win_h = init_win_h;
inline Color bg_color = {0, 0, 0, 255};
inline int target_fps = 60;
inline int is_fps_capped = false;
inline f32 fps, dt = 0;
inline Font font_buf;
inline Font font_ui;

void setup();
void update();
void render();


struct TextBuffer {
    std::string name = EDEX_DEFAULT_BUFFER_NAME;
    std::vector<string> data{};
    bool readonly = false;

    Color bgcolor = Color3(20, 23, 28);
    Vec2 curs_pos = {0, 0};
    uint curs_line = 0;
    uint line_count = 0;

    Font font = {};
    const char* font_family = EDEX_DEFAULT_BUFFER_FONT;
    f32 font_size = 32.0;
    f32 font_glyph_k = 3.0;  // basically: font = (glyhp_size*font_glyph_k) when font size (re)loads
    Color font_clr = Color3(255, 255, 255);
    const f32 font_spacing = font_size / 10.0f;
    f32 char_width = 0;
    f32 ln_wh_k = 1.125;  // line width/height proportion

    enum class TabMode { TAB=1, WS, SOFTWS };
    TabMode tabmode = TabMode::WS;
    uint tab_size = 4;

    // Companion struct for space renderer object
    struct _RendWS {
        bool enabled = true;
        f32 radius = 2.4;
        u_char alpha = 180;
        u_char r = 200;
        u_char g = 200;
        u_char b = 200;
    };
    _RendWS rendws;

    // Companion struct for line padding object
    struct _LinePadding {
        f32 padding = 56;
        f32 fsize = 35;
        f32 fsize_active = 35;
        Color bgcolor = {11, 11, 11, 230};
        Color bgcolor_active = {34, 34, 34, 250};
        Color fcolor = {240, 239, 238, 225};
        Color fcolor_active = {255, 254, 252, 255};
    };
    _LinePadding linepad;

    // Companion struct for line highlighter object
    struct _LnHighlight {
        bool enabled = true;
        f32 thickness = 1;
        f32 thickness_active = 2;
        Color border_color = {220, 220, 220, 80};
        Color border_color_active = {240, 240, 240, 170};
        Color body_color = {10, 10, 10, 15};
        Color body_color_active = {10, 10, 10, 5};
    };
    _LnHighlight ln_highlight;


    std::vector<string>& vec() {
        return this->data;
    }
    std::string& str(uint i) {
        return this->vec()[i];
    }

    /// @arg `foolproof` resizes buffer to wanted
    /// size just to return the char at (i,j)
    char& operator() (uint i, uint j, bool foolproof=true) {
        if (foolproof) {
            if (data.size() <= j)   data.resize(j+1);
            if (data[j].size() <= i) data[j].resize(i+1);
        } else {
            if (j >= data.size() || i >= data[j].size()) {
                static char null_char = '\0';
                return null_char;  // safe fallback
            }
        }
        return data[j][i];
    }

    /// @brief functions starting with 'apply_' below
    /// are executing lambda to buffer's specific data
    /// this one, apply_c, executes @arg action
    /// for each character in the buffer.vec()
    template <typename F>
    void apply_c(F action) {
        for (uint i=0;  i < vec().size();  ++i) {
            string& line = str(i);
            for (uint j=0;  j < line.size();  ++j) {
                action(line[j]);
            }
        }
    }
    /// @brief this one, executes @arg action
    /// for each string in buffer.vec()
    template <typename F>
    void apply_s(F action) {
        for (uint i=0;  i < vec().size();  ++i) {
            action(str(i));
        }
    }
    /// @brief this one, executes @arg action
    /// for each string in buffer.vec too, but
    /// it has access to row and column integers
    template <typename F>
    void apply_sij(F action) {
        for (uint i = 0; i < vec().size(); ++i) {
            string& line = str(i);
            for (uint j = 0; j < line.size(); ++j) {
                action(line[j], i, j);
            }
        }
    }
    /// Print the whole data do @arg _stream
    void print(FILE* _stream=stdout) {
        this->apply_c([_stream](char c) {
            fprintf(_stream, "%c", c);
        });
    }

    /// @brief Delete character at position (col, row)
    bool remove(uint col, uint row) {
        if (this->readonly) {
            log "Can't remove character, buffer is read-only!\n";
            return false;
        } else if (row >= vec().size()) {
            log "Can't remove character, \033[1mrow\033[0;37m"
            "number exceeds the bound!\n"; return false;
        } else if (col >= str(row).size()) {
            log "Can't remove character, \033[1mcol\033[0;37m"
            "number exceeds the bound!\n"; return false;
        }
        str(row).erase(str(row).begin() + col);
        log "Deleted char at (" << col << ", " << row << ")\n";
        return true;
    }

    /// @brief Delete line at position (row)
    bool remove_line(uint row) {
        if (row > vec().size()) {
            log "Can't remove line, \033[1mrow\033[37m"
            "number exceeds the bound!\n";
            return false;
        }
        vec().erase(vec().begin() + row);
        return true;
    }
    
    /// @brief Insert character at position (col, row)
    void insert(char c, uint col, uint row) {
        if (row >= vec().size()) return;
        if (col > str(row).size()) col = str(row).size();

        str(row).insert(str(row).begin() + col, c);
        log "Inserted '" << c << "' at (" << col << ", " << row << ")\n";
    }

    /// @brief based on the settings, such, insert tab @arg c times
    /// @return total steps done, @usage: cur.step(buf.insert_tab(x,y))
    uint insert_tab(uint col, uint row, uint c=1) {
        if (c <= 0) {
            log "You can't insert a tab less than 1!\n";
            return 0;
        }
        // get total steps that will be done
        uint space_c = this->tab_size;
        const uint steps = c*space_c;
        while (c--)
        {
            space_c = this->tab_size;
            while (space_c--)
            {
                if (tabmode==TabMode::TAB) {
                    this->insert('\t', col, row);
                } else if (tabmode==TabMode::WS || tabmode==TabMode::SOFTWS) {
                    this->insert(' ', col, row);
                }
            }
        }
        return steps;
    }


    /// @brief set new font size
    /// @return old font size
    f32 setFontSize(f32 new_size) {
        if (new_size < EDEX_FONT_SIZE_MIN) {
            log "Can't set font size below this: " << EDEX_FONT_SIZE_MIN
            << "\n"; return font_size;
        } else if (FEQ( new_size , font_size )) {
            log "Font size unchanged!\n"; return font_size;
        }

        f32 size_old = font_size;
        Font font_new = LoadFontEx(
            font_family, CAST(int, new_size * font_glyph_k+0.5f),
            nullptr, 0
        );
        Font font_old = this->font;
        this->font = font_new;
        
        this->font_size = new_size;
        UnloadFont(font_old);
        char_width = MeasureTextEx(font, "A", font_size, font_spacing).x;
        return size_old;
    }


    // pass nothing or true if it is the active line, else pass false
    void renderLines() {
        for (uint i=0;  i < data.size();  ++i) {
            // pad object - width & height of the line padding
            Vec2 pad = {0+linepad.padding, font_size*ln_wh_k};
            bool active = i==this->curs_line;

            // Draw : Line paddings
            DrawRectangle(
                0, i*pad.y,
                pad.x, pad.y,
                ((active))? (linepad.bgcolor_active) : (linepad.bgcolor)
            );

            // Draw : Line numbers
            renderText(
                font, itos(i+1),
                {pad.x-(linepad.padding/2)-(linepad.fsize/2), pad.y*i},
                ((active))? (linepad.fcolor_active) : (linepad.fcolor),
                ((active))? (linepad.fsize_active) : (linepad.fsize),
                1.0  // spacing
            );

            // Draw : Line highlights
            if (ln_highlight.enabled)  {
                // Draw : .body
                DrawRectangle(
                    linepad.padding, pad.y*i,
                    win_w-linepad.padding, font_size*ln_wh_k,
                    ((active))? (ln_highlight.body_color_active) : (ln_highlight.body_color)
                );
                // Draw : .borders
                DrawRectangleLinesEx(
                    Rect {
                        linepad.padding, pad.y*i,
                        win_w-linepad.padding, font_size*ln_wh_k
                    },
                    ((active))? (ln_highlight.thickness_active) : (ln_highlight.thickness),
                    ((active))? (ln_highlight.border_color_active) : (ln_highlight.border_color)
                );
            }
        }
    }

    /// FILE OPERATIONS
    bool fsave(const string& dir, const string& opt_name = "") {
        string filename = opt_name.empty() ? this->name : opt_name;
        if (filename.empty()) {
            log "\033[31mfsave: no filename specified or buffer has no name!"
            "\033[0m\n"; return false;
        }
        string full_path = dir + "/" + filename;
        std::ofstream of(full_path);
        if (!of.is_open()) {
            log "\033[31mfsave: failed to open file: " <<
            full_path << "\033[0m\n";
            return false;
        }
        for (auto& line : this->data) {
            of << line << "\n";
        }
        logx "\033[34mSaved file to " << full_path << "\033[0m" << logxe;
        return true;
    }


    void reload() {
        char_width = MeasureTextEx(font, "A", font_size, font_spacing).x;
    }

    void start() {
        vec().resize(1);
        this->font = LoadFontEx(
            font_family, CAST(int, font_size * font_glyph_k+0.5f),
            nullptr, 0
        );
        this->reload();
    }

    void loop(Vec2 _cursor_position, uint _cursor_line_number) {
        curs_pos = _cursor_position;
        curs_line = _cursor_line_number;
        line_count = this->vec().size();
    }

    void draw()
    {
        // Draw background
        DrawRectangle(
            0, 0,
            win_w, win_h,
            bgcolor
        );

        // Render characters
        this->apply_sij([this](char c, uint row, uint col){
            Vec2 char_pos = {
                linepad.padding + (col * char_width),
                row * font_size*ln_wh_k
            };
            if (c==' ' && rendws.enabled) {
                DrawCircle(
                    char_pos.x+2*font_spacing, char_pos.y+font_size/2,
                    rendws.radius, {rendws.r, rendws.g, rendws.b, rendws.alpha}
                );
            } else {
                renderText(
                    font, c, char_pos, font_clr,
                    font_size, font_spacing
                );
            }
        });

        renderLines();
    }
};

