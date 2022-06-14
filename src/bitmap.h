#ifndef BITMAP_H
#define BITMAP_H

#include "general.h"

enum Texture_Format {
    TEXTURE_FORMAT_RGBA8,
    TEXTURE_FORMAT_RGB8,
};

struct Bitmap {
    int width;
    int height;
    Texture_Format format;
    int channels;
    
    unsigned char *data;

    void load_from_file(char *file_path);
    u32 get_argb(int x, int y);
    u32 get_rgb(int x, int y);
};

#endif
