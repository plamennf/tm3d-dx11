#include "bitmap.h"

#include <stb_image.h>

void Bitmap::load_from_file(char *file_path) {
    stbi_set_flip_vertically_on_load(true);
    data = stbi_load(file_path, &width, &height, &channels, 0);
    if (channels == 4) {
        format = TEXTURE_FORMAT_RGBA8;
    } else if (channels == 3) {
        format = TEXTURE_FORMAT_RGB8;
    }
}

u32 Bitmap::get_argb(int x, int y) {
    u32 bytes_per_pixel = channels;
    u8 *pixel_offset = data + ((y * width + x) * bytes_per_pixel);
    u8 r = pixel_offset[0];
    u8 g = pixel_offset[1];
    u8 b = pixel_offset[2];
    u8 a = channels == 4 ? pixel_offset[3] : 0xff;
    return (a << 24) | (r << 16) | (g << 8) | (b << 0);
}

u32 Bitmap::get_rgb(int x, int y) {
    u32 bytes_per_pixel = channels;
    u8 *pixel_offset = data + ((y * width + x) * bytes_per_pixel);
    u8 r = pixel_offset[0];
    u8 g = pixel_offset[1];
    u8 b = pixel_offset[2];
    u8 a = 0x00;
    return (a << 24) | (r << 16) | (g << 8) | (b << 0);    
}
