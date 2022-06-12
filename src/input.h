#ifndef INPUT_H
#define INPUT_H

struct Key_Info {
    bool is_down;
    bool was_down;
    bool changed;
};

enum Key {
    KEY_UNKNOWN,

    KEY_A,
    KEY_D,
    KEY_W,
    KEY_S,
    
    KEY_SPACE,
    
    KEY_F11,
    KEY_ESCAPE,
    KEY_ENTER,

    KEY_LEFT,
    KEY_RIGHT,
    KEY_UP,
    KEY_DOWN,
    
    NUM_KEYS,
};

bool is_key_down(Key key);
bool is_key_pressed(Key key);
bool was_key_pressed(Key key);
int get_mouse_pointer_delta_x();
int get_mouse_pointer_delta_y();

#endif
