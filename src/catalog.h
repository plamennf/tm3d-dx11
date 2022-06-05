#ifndef CATALOG_H
#define CATALOG_H

struct Texture_Map;

Texture_Map *find_or_create_texture(char *short_name);

#endif
