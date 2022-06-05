#ifndef BITMAP_H
#define BITMAP_H

enum Texture_Format {
    TEXTURE_FORMAT_RGBA8,
    TEXTURE_FORMAT_RGB8,
};

struct Bitmap {
    int width;
    int height;
    Texture_Format format;

    unsigned char *data;
};

#endif
