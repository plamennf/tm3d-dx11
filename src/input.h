#ifndef INPUT_H
#define INPUT_H

struct Key_Info {
    bool is_down;
    bool was_down;
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
    
    NUM_KEYS,
};

bool is_key_down(Key key);
bool is_key_pressed(Key key);

#endif
