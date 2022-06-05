#ifndef OS_H
#define OS_H

#include "core.h"

struct Time {
    u32 hour;
    u32 minute;
    u32 second;
};

Time os_get_system_time();
Time os_get_local_time();

double os_get_time();

char *os_read_entire_file(char *filepath, s64 *out_length = nullptr);
bool os_file_exists(char *filepath);

void os_poll_events();

char *os_getcwd();
void os_setcwd(char *dir);

char *os_get_path_to_executable();

#endif
