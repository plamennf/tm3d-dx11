#ifndef OS_H
#define OS_H

#include "general.h"

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
void os_get_last_write_time(char *file_path, u64 *out_time);

void os_poll_events();

char *os_get_path_to_executable();
void os_setcwd(char *dir);

void os_hide_cursor();
void os_show_cursor();

void os_get_mouse_pointer_position(int *x, int *y, bool flipped = true);

#endif
