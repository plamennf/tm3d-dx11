#include <ft2build.h>
#include FT_FREETYPE_H

#include "font.h"
#include "draw.h"
#include "array.h"

static bool ft_initted;
static FT_Library ft;
static Array <Font *> loaded_fonts;

Font *load_font(char *full_path, int size) {
    Font *result = new Font();

    if (!ft_initted) {
        FT_Init_FreeType(&ft);
        ft_initted = true;
    }

    FT_New_Face(ft, full_path, 0, &result->face);
    FT_Set_Pixel_Sizes(result->face, 0, size);

    result->has_kerning = FT_HAS_KERNING(result->face);

    result->character_height = size;

    result->bx = result->by = 0;
    result->bw = result->bh = 1024;

    Bitmap bitmap = {};
    bitmap.width = result->bw;
    bitmap.height = result->bh;
    bitmap.format = TEXTURE_FORMAT_RGBA8;

    result->map = create_texture(bitmap);
    
    return result;
}

Font *get_font_at_size(char *short_name, int size) {
    for (int i = 0; i < loaded_fonts.count; i++) {
        Font *font = loaded_fonts[i];
        if (strings_match(font->short_name, short_name) && font->character_height == size) {
            return font;
        }
    }

    char *full_path = mprintf("data/fonts/%s", short_name);
    
    Font *result = load_font(full_path, size);
    result->full_path = full_path;
    result->short_name = copy_string(short_name);
    loaded_fonts.add(result);

    return result;
}

Glyph *get_or_load_glyph(Font *font, int codepoint) {
    Glyph *glyph = nullptr;
    bool glyph_exists = font->glyphs.contains(codepoint);
    if (!glyph_exists) {
        font->glyphs.add(codepoint);
    }
    glyph = &font->glyphs.get(codepoint);
    
    if (glyph->height == font->character_height) return glyph;

    unsigned long glyph_index = FT_Get_Char_Index(font->face, codepoint);
    FT_Load_Glyph(font->face, glyph_index, FT_LOAD_RENDER);

    glyph->advance = font->face->glyph->advance.x >> 6;
    glyph->bearing_x = font->face->glyph->bitmap_left;
    glyph->bearing_y = font->face->glyph->bitmap_top;
    
    glyph->height = font->character_height;
    
    if (codepoint == ' ' || codepoint == '\n' || codepoint == '\t') return glyph;
    
    glyph->size_x = font->face->glyph->bitmap.width;
    glyph->size_y = font->face->glyph->bitmap.rows;

    glyph->min_uv = make_vector2((float)font->bx, (float)font->by);
    glyph->max_uv = glyph->min_uv + make_vector2((float)glyph->size_x, (float)glyph->size_y);

    glyph->min_uv.x /= (float)font->bw; glyph->min_uv.y /= (float)font->bh;
    glyph->max_uv.x /= (float)font->bw; glyph->max_uv.y /= (float)font->bh;

    u8 *data = new u8[glyph->size_x * glyph->size_y * 4];
    defer { delete [] data; };
    for (int y = 0; y < glyph->size_y; y++) {
        for (int x = 0; x < glyph->size_x; x++) {
            u8 source = font->face->glyph->bitmap.buffer[y * glyph->size_x + x];
            u8 *dest = &data[(y * glyph->size_x + x) * 4];

            dest[0] = 255;
            dest[1] = 255;
            dest[2] = 255;
            dest[3] = source;
        }
    }

    update_texture(font->map, font->bx, font->by, glyph->size_x, glyph->size_y, data);

    font->bx += glyph->size_x + 4;

    if (font->bx >= font->bw) {
        font->by += font->character_height;
        font->bx = 0;
    }

    return glyph;
}

int get_kerning_in_pixels(Font *font, int codepoint, int next_codepoint) {
    unsigned long glyph_index = FT_Get_Char_Index(font->face, codepoint);
    unsigned long next_glyph_index = FT_Get_Char_Index(font->face, next_codepoint);

    FT_Vector kern;
    FT_Get_Kerning(font->face, glyph_index, next_glyph_index, FT_KERNING_DEFAULT, &kern);

    return kern.x >> 6;
}

int get_string_width_in_pixels(Font *font, char *text) {
    if (!text) return 0;
    
    int width = 0;
    for (char *at = text; *at;) {
        int codepoint_byte_count = 0;
        int codepoint = get_codepoint(at, &codepoint_byte_count);
        Glyph *glyph = get_or_load_glyph(font, codepoint);
        
        width += glyph->advance;

        if (font->has_kerning) {
            int next_codepoint_byte_count = 0;
            int next_codepoint = get_codepoint(at + codepoint_byte_count, &next_codepoint_byte_count);
            width += get_kerning_in_pixels(font, codepoint, next_codepoint);
        }
        
        at += codepoint_byte_count;
    }
    
    return width;
}
