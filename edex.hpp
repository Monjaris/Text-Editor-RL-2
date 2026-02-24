#pragma once
#include "utils.h"

#define log std::cerr<<
#define logx std::cerr << "\033[1;34m" <<
#define logxe "\033[0m\n"

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
    std::string name;
    std::vector<string> data{};

    Vec2 curs_pos = {0, 0};
    Font font;
    Color font_clr = Color3(255, 255, 255);
    f32 font_size = 32.0f;
    const f32 font_spacing = font_size / 10.0f;
    f32 char_width = 0;
    f32 ln_wh_k = 1.15;  // line width/height proportion

    // Companion struct for space renderer object
    struct _RendWS {
        bool enabled = true;
        f32 radius;
        u_char alpha = 120;
        u_char r = 255;
        u_char g = 255;
        u_char b = 255;
    };
    _RendWS rendws;


    std::vector<string>& vec() {
        return this->data;
    }
    std::string& str(uint i) {
        return this->vec()[i];
    }

    char& operator() (uint i, uint j) {
        size_t vsize = data.size();
        size_t hsize = data[j].size();
        if (vsize <= j) {
            data.resize(j+1);
        }
        if (hsize <= i) {
            data[j].resize(i+1);
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
            string line = str(i);
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

    /// @brief Delete character at position (col, row)
    /// @return true if deleted, false if nothing to delete
    bool remove(uint col, uint row) {
        if (row >= vec().size()) return false;
        if (col >= str(row).size()) return false;

        str(row).erase(str(row).begin() + col);
        log "Deleted char at (" << col << ", " << row << ")\n";
        return true;
    }
    
    /// @brief Insert character at position (col, row)
    void insert(char c, uint col, uint row) {
        if (row >= vec().size()) return;
        if (col > str(row).size()) col = str(row).size();

        str(row).insert(str(row).begin() + col, c);
        log "Inserted '" << c << "' at (" << col << ", " << row << ")\n";
    }

    void print(FILE* _stream=stdout) {
        this->apply_c([_stream](char c) {
            fprintf(_stream, "%c", c);
        });
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


    void start() {
        vec().resize(1);
        font = LoadFont(EDEX_DEFAULT_BUFFER_FONT);
        char_width = MeasureTextEx(font, "A", font_size, font_spacing).x;
    }
    void loop(Vec2 _cursor_position) {
        curs_pos = _cursor_position;
    }
    void draw() {
        this->apply_sij([this](char c, uint row, uint col){
            // log "\rRendering character '" << line[j] << "'\n";
            Vec2 char_pos = {
                col * char_width,
                row * font_size*ln_wh_k
            };
            if (c==' ' && rendws.enabled)
            {
                DrawCircle(
                    char_pos.x+2*font_spacing, char_pos.y+font_size/2,
                    rendws.radius, {rendws.r, rendws.g, rendws.b, rendws.alpha}
                );
                goto bypass;
            }

            renderText(
                font, c, char_pos, font_clr,
                font_size, font_spacing
            );

        bypass:
            //
        });
    }
};

