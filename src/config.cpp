#include "general.h"
#include "config.h"
#include "text_file_handler.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

Config load_config() {
    Config result = {};

    Text_File_Handler handler;
    handler.start_file(CONFIG_FILEPATH, CONFIG_FILEPATH, "config");
    if (handler.failed) {
        return result;
    }
    
    while (1) {
        char *line = handler.consume_next_line();
        if (!line) break;

        if (starts_with(line, "render_scale")) {
            line += get_string_length("render_scale");
            line = eat_spaces(line);
            result.render_scale = atof(line);
        }
    }
    
    return result;
}

void save_config() {
    FILE *file = fopen(CONFIG_FILEPATH, "wt");
    if (!file) {
        printf("[config] Unable to open file '%s' for writing\n", CONFIG_FILEPATH);
        return;
    }
    defer { fclose(file); };

    fprintf(file, "[%d]\n\n", Config::VERSION);
    
    extern float render_scale_to_draw; // From menu.cpp
    fprintf(file, "render_scale %f\n", render_scale_to_draw);
}
