#ifndef DISPLAY_H
#define DISPLAY_H

void display_init(int width, int height, char *title);

int display_get_width();
int display_get_height();
bool display_is_open();
void *display_get_native_window();

#endif
