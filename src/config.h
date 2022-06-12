#pragma once

#define CONFIG_FILEPATH "settings.cfg"

struct Config {
    static const int VERSION = 1;
    
    float render_scale = 1.0f; // @v1
};

Config load_config();
void save_config();
