#include "catalog.h"

#include "draw.h"
#include "os.h"
#include "array.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

static Array <Texture_Map *> loaded_textures;

Texture_Map *find_or_create_texture(char *short_name) {
    for (int i = 0; i < loaded_textures.count; i++) {
        Texture_Map *map = loaded_textures[i];
        if (strings_match(map->short_name, short_name)) {
            return map;
        }
    }

    char *extensions[] = {
        "png",
        "jpg",
        "bmp",
    };
    
    char *full_path = nullptr;
    for (int i = 0; i < ArrayCount(extensions); i++) {
        full_path = mprintf("data/textures/%s.%s", short_name, extensions[i]);
        if (os_file_exists(full_path)) {
            break;
        } else {
            delete [] full_path;
            full_path = nullptr;
        }
    }

    if (!full_path) return nullptr;
    
    int width, height, channels;
    stbi_set_flip_vertically_on_load(true);
    u8 *data = stbi_load(full_path, &width, &height, &channels, 0);
    if (!data) {
        delete [] full_path;
        return nullptr;
    }
    defer { stbi_image_free(data); };
    
    Bitmap bitmap = {};
    bitmap.width = width;
    bitmap.height = height;

    if (channels == 4) {
        bitmap.format = TEXTURE_FORMAT_RGBA8;
    } else if (channels == 3) {
        bitmap.format = TEXTURE_FORMAT_RGB8;
    }

    bitmap.data = data;
    
    Texture_Map *map = create_texture(bitmap);
    map->full_path = full_path;
    map->short_name = copy_string(short_name);
    loaded_textures.add(map);
    return map;
}
