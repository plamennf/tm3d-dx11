#ifndef FONT_H
#define FONT_H

#include "geometry.h"
#include "hash_table.h"

struct Texture_Map;

struct Glyph {
    int height;
    int size_x, size_y;
    int bearing_x, bearing_y;
    Vector2 min_uv, max_uv;
    u32 advance;
    Texture_Map *map;
};

struct Font {
    char *full_path;
    char *short_name;

    struct FT_FaceRec_ *face;
    Hash_Table <int, Glyph> glyphs;
    int character_height;

    bool has_kerning;

    int bx, by, bw, bh;
    Texture_Map *map;
};

Font *load_font(char *short_name, int size);
Font *get_font_at_size(char *short_name, int size);
Glyph *get_or_load_glyph(Font *font, int codepoint);
int get_kerning_in_pixels(Font *font, int codepoint, int next_codepoint);
int get_string_width_in_pixels(Font *font, char *text);

#endif
